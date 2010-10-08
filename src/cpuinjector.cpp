/*
 * This file is part of KRASH.
 *
 *  KRASH is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  KRASH is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with KRASH.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright Swann Perarnau, 2008
 *  Contact: firstname.lastname@imag.fr
 */
#include "cpuinjector.hpp"
#include "events.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <typeinfo>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE // need by sched.h
#endif
#include <sched.h> // for affinity
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

namespace cpuinjector {

// STANDARD NAMES FOR CGROUPS
static char CPU_CGROUP_NAME[] = "cpu";
//static char CPUSET_CGROUP_NAME[] = "cpuset";
static char CPU_SHARES[] = "cpu.shares";

// PUBLIC VARIABLES
std::string cpu_cgroup_root = "/";
std::string cpuset_cgroup_root = "/";
std::string alltasks_groupname = "alltasks";
std::string cgroups_basename = "krash.";

// STATIC INTERNAL VARIABLES
static struct cgroup *all_cg = NULL;
static std::map< unsigned int, struct cgroup*> burners_cgs;
static std::map< unsigned int, pid_t > burners_pids;
static bool error_on_ev_child = true;
static std::map< unsigned int, ev::child * > burners_watchers;
static std::map< pid_t, unsigned int > burners_cpus;

/* ERROR POLICY:
 * The fact that libcgroup can fail in a _lot_ of ways means
 * we need to recover errors in a consistent way and let the cpuinjector caller
 * call the cleaner before exiting.
 * Every public method should return 0 or 1 (other values are for us).
 * Every internal method should clean what it created but not exit.
 * Each method should call cg_error before returning, this way we can trace the call stack
 * A special method is here to do the _big_ cleanup (like child processes and stuff)
 */


/* helper macro for cgroup errors */
#define cg_error(errno) {	\
	if(errno == 0) {	\
		std::cerr << "Internal Error: called error handling on good condition, report this bug !" << " File: " << __FILE__ << ":" << __LINE__ << std::endl;	\
	} else if(errno == 1) {	\
		std::cerr << "Internal Error: unrecoverable error (probably a failed malloc) !" << " File: " << __FILE__ << ":" << __LINE__<< std::endl;	\
	}	\
	else {	\
		std::cerr << "Error: " << cgroup_strerror(errno)<< " File: " << __FILE__ << ":" << __LINE__<< std::endl;	\
	}	\
}

#define TEST_FOR_ERROR(err,label) {	\
	if(err) {			\
		cg_error(err);	\
		goto label;		\
	}				\
}					\

#define TEST_FOR_ERROR_IGNORE(err,ignore,label) {	\
	if(err && err != ignore) {			\
		cg_error(err);				\
		goto label;				\
	}						\
}							\

#define TEST_FOR_ERROR_P(p,err,label) {	\
	if(!p) {			\
		err = 1;		\
		cg_error(err);		\
		goto label;		\
	}				\
}					\

// register something went wrong but don't exit
#define SAVE_RET(err,ret) if(err) { ret = err; }


void child_callback(ev::child &w, int revents) {
	unsigned int cpu;
	cpu = burners_cpus[w.rpid];
	burners_pids.erase(cpu);
	burners_watchers[cpu]->stop();
	delete burners_watchers[cpu];
	if(error_on_ev_child)
	{
		std::cerr << "Error: child " << w.pid << " exited with status "
			<< w.rstatus << std::endl;
				events::stop(1);
	}
}

// kill and wait a task
static int kill_wait(pid_t task) {
	int err;
	error_on_ev_child = false;
	err = kill(task,SIGKILL);
	if(err)
		std::cerr << "Error: " << strerror(err) << " in " << __FILE__ << ":" << __LINE__ << std::endl;
	return err;
}

int setup(ActionsList& list) {
	int err;
	std::set<unsigned int> cpus;
	std::set<unsigned int>::iterator it;
	ActionsList copy;

	err = setup_system();
	TEST_FOR_ERROR(err,error);

	/* we need to copy the list for its inspection */
	copy = list;
	while(!copy.empty()) {
		Action *a = copy.top();
		if(typeid(*a) == typeid(CPUAction)) {
			CPUAction *ca = (CPUAction *)a;
			cpus.insert(ca->cpu);
		}
		copy.pop();
	}
	/* iterate through cpus to setup them */
	for(it = cpus.begin(); it != cpus.end(); it++) {
		err = setup_cpu(*it);
		TEST_FOR_ERROR(err,error);
	}
	return 0;
error:
	return err;
}

int create_group(struct cgroup** ret, std::string name, u_int64_t shares) {
	int err;
	struct cgroup* cg;
	struct cgroup_controller *cgc;

	std::string full_path = cpu_cgroup_root + name;
	cg = cgroup_new_cgroup(full_path.c_str());
	TEST_FOR_ERROR_P(cg,err,error);

	/* to gather data on it, we must add the cpu controller and
	 * its cpu.shares control file */
	cgc = cgroup_add_controller(cg,CPU_CGROUP_NAME);
	TEST_FOR_ERROR_P(cgc,err,error_free);

	/* 1024 is default and a good value */
	err = cgroup_add_value_uint64(cgc,CPU_SHARES,shares);
	TEST_FOR_ERROR(err,error_free);

	/* create the group on the filesystem, populating the cg
	 * with current values */
	err = cgroup_create_cgroup(cg,0);
	TEST_FOR_ERROR(err,error_free);

	*ret = cg;
	return 0;
error_free:
	cgroup_free(&cg);
error:
	cg_error(err);
	*ret = NULL;
	return err;
}

int setup_system() {
	void *handle = NULL;
	pid_t pid;
	int err;

	/* init cgroup library */
	err = cgroup_init();
	TEST_FOR_ERROR(err,error);

	/* create the alltasks group */
	err = create_group(&all_cg,alltasks_groupname,1024);
	TEST_FOR_ERROR(err,error);

	/* migrate all tasks, using task walking functions */
	// ok libcgroup is stupid so I need to pass it a char* and I
	// only have a const char*, lets do stupid things !
	err = cgroup_get_task_begin(&cpu_cgroup_root[0],CPU_CGROUP_NAME,&handle,&pid);
	if(err == ECGEOF)
		goto end_tasks; // TODO: maybe this is bad, at least one task should be moved, no ?

	TEST_FOR_ERROR(err,error_free);

	do {
		/* WE MUST SKIP ERRORS HERE, NOT ALL TASKS CAN BE MOVED IF
		 * OUR ROOT IS THE REAL ROOT */
		cgroup_attach_task_pid(all_cg,pid); // TODO: skip getpid()
	}
	while((err = cgroup_get_task_next(&handle,&pid)) == 0);
	TEST_FOR_ERROR_IGNORE(err,ECGEOF,error_free);

end_tasks:
	cgroup_get_task_end(&handle);
	return 0;

error_free:
	// create_group() already cleans all_cg so just use this to delete the group
	cgroup_delete_cgroup(all_cg,0);
	cgroup_free(&all_cg);
	// if this fails, we must ensure all_cg is NULL
	all_cg = NULL;
error:
	return err;
}

int setup_cpu(unsigned int cpuid) {
	/* what we need to do:
	 * - create a cpu control group
	 * - fork a process
	 * - make it a member of this group
	 * - restrict it to the cpu
	 * - register this cpu as active
	 */
	int err;
	struct cgroup *burner;
	std::string burner_name;
	pid_t burner_pid;

	/* create the burner cpu cgroup */
	burner_name = cgroups_basename + itos(cpuid);
	err = create_group(&burner,burner_name,1024);
	TEST_FOR_ERROR(err,error);

	/* fork a burner process */
	burner_pid = fork();
	if(!burner_pid) { // never leave this part
		/* attach myself to a cpu */
		cpu_set_t mycpuset;
		CPU_ZERO(&mycpuset);
		CPU_SET(cpuid,&mycpuset);
		err = sched_setaffinity(0,sizeof(cpu_set_t),&mycpuset);
		/* loop until the end of time */
		while(1);
	}

	/* attach pid to the burner group */
	err = cgroup_attach_task_pid(burner,burner_pid);
	TEST_FOR_ERROR(err,error_free);

	burners_cgs[cpuid] = burner;
	burners_pids[cpuid] = burner_pid;
	burners_watchers[cpuid] = new ev::child(*events::loop);
	burners_watchers[cpuid]->set<child_callback>(NULL);
	burners_watchers[cpuid]->set(burner_pid,0);
	burners_watchers[cpuid]->start();
	return 0;

error_free: // we do not recover from this, too hard
	kill_wait(burner_pid);
	cgroup_delete_cgroup(burner,0);
	cgroup_free(&burner);
error:
	return err;
}

int apply_share(unsigned int cpuid, unsigned int share) {
	/* what we need to do
	 * - know the priority of the alltask group
	 * - compute the priority to apply to a given cpu group
	 * - apply it
	 */
	int err;
	double s,a,nprio;
	struct cgroup *burner;
	struct cgroup_controller *all_cgc,*burner_cgc;
	u_int64_t all_prio,new_prio;

	/* retrieve the cpu cgroup for all and get the cpu.shares value */
	all_cgc = cgroup_get_controller(all_cg,CPU_CGROUP_NAME);
	TEST_FOR_ERROR_P(all_cgc,err,error);

	err = cgroup_get_value_uint64(all_cgc,CPU_SHARES,&all_prio);
	TEST_FOR_ERROR(err,error);

	/* get the burner cgroup */
	burner = burners_cgs[cpuid];

	/* compute share
	 * Here is the value we seek: new_prio = (share/100) * (new_prio + all_prio)
	 * The real computation is derived from it.
	 * We do computations in floats for better precision.
	 */
	// adjust share because our formula does not allows the value 100
	if(share == 100)
		share = 99;

	s = share;
	a = all_prio;
	nprio = (a*s/100.0) * (100.0 / (100.0 - s));
	new_prio = nprio;

	/* get the cpu controller */
	burner_cgc = cgroup_get_controller(burner,CPU_CGROUP_NAME);
	TEST_FOR_ERROR_P(burner_cgc,err,error);

	/* set the new value */
	err = cgroup_set_value_uint64(burner_cgc,CPU_SHARES,new_prio);
	TEST_FOR_ERROR(err,error);

	/* force lib to rewrite values */
	err = cgroup_modify_cgroup(burner);
	TEST_FOR_ERROR(err,error);

	return 0;
error:
	return err;
}

// we get called with every exit, on success AND on failure
// so we must check for each component to ensure everything is in good shape
int cleanup() {
	int err;
	int ret= 0;
	// cleanup all_cg
	if(all_cg) {
		// just delete the group and free the structure
		// we can't recover from this
		err = cgroup_delete_cgroup(all_cg,0);
		SAVE_RET(err,ret);
		cgroup_free(&all_cg);
	}

	// cleanup burners
	std::map<unsigned int, struct cgroup*>::iterator it;
	for(it = burners_cgs.begin(); it != burners_cgs.end(); it++) {
		struct cgroup *cg = it->second;

		// find the pid of the burner
		std::map<unsigned int, pid_t>::iterator b;
		b = burners_pids.find(it->first);
		if(b != burners_pids.end())
		{
			// kill and wait him
			err = kill_wait(b->second);
			SAVE_RET(err,ret);

		}
		// free the structure
		err = cgroup_delete_cgroup(cg,0);
		SAVE_RET(err,ret);
		cgroup_free(&cg);
	}
	return ret;
}

} // end namespace

// CPUAction implementation

/* class CPUAction */
CPUAction::CPUAction(std::string id, unsigned int time, unsigned int cpu, unsigned int load) : Action(id,time) {
	this->cpu = cpu;
	this->load = load;
}

CPUAction::CPUAction(unsigned int time, unsigned int cpu, unsigned int load) : Action(std::string("cpu"),time) {
	this->cpu = cpu;
	this->load = load;
	this->id += itos(cpu);
}

int CPUAction::activate() {
	int err;
	std::cout << "Applying share " << this->load << " on cpu " << this->cpu << " asked for time " << this->time << std::endl;
	err = cpuinjector::apply_share(this->cpu,this->load);
	return err;
}


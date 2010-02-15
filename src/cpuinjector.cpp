#include "cpuinjector.hpp"
#include "utils.hpp"
#include <libcgroup.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE // need by sched.h
#endif
#include <sched.h> // for affinity
#include <sys/types.h>

/* helper macro for cgroup errors */
#define cg_error(errno) {	\
	if(errno == 0) {	\
		std::cerr << "Internal Error: called error handling on good condition, report this bug !" << " File: " << __FILE__ << ":" << __LINE__ << std::endl;	\
	} else if(errno == ECGOTHER) {	\
		std::cerr << "Error: " << strerror(cgroup_get_last_errno())<< " File: " << __FILE__ << ":" << __LINE__<< std::endl;	\
	}	\
	else {	\
		std::cerr << "Error: " << cgroup_strerror(errno)<< " File: " << __FILE__ << ":" << __LINE__<< std::endl;	\
	}	\
}

CPUInjector *MainCPUInjector;

CPUInjector::CPUInjector(std::string cpu_cg_root,std::string cpuset_cg_root,std::string all_name,std::string cg_basename,unsigned int all_prio) {
	cpu_cgroup_root = cpu_cg_root;
	cpuset_cgroup_root = cpuset_cg_root;
	alltasks_groupname = all_name;
	cgroups_basename = cg_basename;
	alltasks_priority = all_prio;
}


int CPUInjector::setup(ActionsList& list) {
	int err;
	err = setup_system();
	if(err) {
		return err;
	}
	/* we need to copy the list for its inspection */
	ActionsList copy(list);
	std::set<unsigned int> cpus;
	while(!copy.empty()) {
		Action *a = copy.top();
		if(a->get_id().find(CPU_CGROUP_NAME) != std::string::npos) {
			CPUAction *ca = (CPUAction *)a;
			cpus.insert(ca->get_cpu());
		}
		copy.pop();
	}
	/* iterate through cpus to setup them */
	std::set<unsigned int>::iterator it;
	for(it = cpus.begin(); it != cpus.end(); it++) {
		err = setup_cpu(*it);
		if(err) {
			std::cerr << "Error: failed to init cpu "<< *it << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}


int CPUInjector::setup_system() {
	struct cgroup *alltasks;
	void *handle = NULL;
	pid_t pid;
	int err;

	/* init cgroup library */
	err = cgroup_init();
	if(err)
	{
		cg_error(err);
		exit(EXIT_FAILURE);
	}

	/* create the /alltasks group */
	std::string all_path = cpu_cgroup_root + alltasks_groupname;
	alltasks = cgroup_new_cgroup(all_path.c_str());
	if(!alltasks)
		return 1;

	cgroup_add_controller(alltasks,CPU_CGROUP_NAME);

	err = cgroup_create_cgroup(alltasks,0);
	if(err) {
		cg_error(err);
		exit(EXIT_FAILURE);
	}

	/* migrate all tasks, using task walking functions */
	// ok libcgroup is stupid so I need to pass it a char* and I
	// only have a const char*, lets do stupid things !
	err = cgroup_get_task_begin(&cpu_cgroup_root[0],CPU_CGROUP_NAME,&handle,&pid);
	if(err) {
		if(err == ECGEOF)
			return 0;
		cg_error(err);
		exit(EXIT_FAILURE);
	}

	do {
		cgroup_attach_task_pid(alltasks,pid);
		// TODO: skip getpid()

	}
	while((err = cgroup_get_task_next(&handle,&pid)) == 0);
	if(err != ECGEOF) {
		cg_error(err);
		exit(EXIT_FAILURE);
	}
	cgroup_get_task_end(&handle);
	return 0;
}

int CPUInjector::setup_cpu(unsigned int cpuid) {
	/* what we need to do:
	 * - create a cpu control group
	 * - fork a process
	 * - make it a member of this group
	 * - restrict it to the cpu
	 * - register this cpu as active
	 */

	/* create the burner cpu cgroup */
	std::string burner_path = cpu_cgroup_root + cgroups_basename + itos(cpuid);
	struct cgroup *burner = cgroup_new_cgroup(burner_path.c_str());
	if(!burner) {
		return 1;
	}
	int err;
	cgroup_add_controller(burner,CPU_CGROUP_NAME);

	err = cgroup_create_cgroup(burner,0);
	if(err) {
		cg_error(err);
		exit(EXIT_FAILURE);
	}

	/* fork a burner process */
	pid_t burner_pid = fork();
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
	if(err) {
		cg_error(err);
		// TODO: clean allocated structures and processes
		return 1;
	}

	/* register cpu as active */
	active_cpus.insert(std::pair<unsigned int, pid_t>(cpuid,burner_pid));
	return 0;
}

int CPUInjector::apply_share(unsigned int cpuid, unsigned int share) {
	/* what we need to do
	 * - know the priority of the alltask group
	 * - compute the priority to apply to a given cpu group
	 * - apply it
	 */
	std::string burner_path = cpu_cgroup_root + cgroups_basename + itos(cpuid);
	struct cgroup *burner = cgroup_new_cgroup(burner_path.c_str());
	if(!burner) {
		return 1;
	}

	/* get the cgroup, forcing the lib to fill internals with real values */
	int err;
	err = cgroup_get_cgroup(burner);
	if(err) {
		cg_error(err);
		exit(EXIT_FAILURE);
	}
	/* get the cpu controller */
	struct cgroup_controller *cg_cpu = cgroup_get_controller(burner,CPU_CGROUP_NAME);
	if(!cg_cpu)
		return 1;


	/* get the current value */
	u_int64_t val;
	err = cgroup_get_value_uint64(cg_cpu,"cpu.shares",&val);
	if(err) {
		cg_error(err);
		exit(EXIT_FAILURE);
	}

	/* compute share */
	u_int64_t newprio = val;

	/* set the new value */
	err = cgroup_set_value_uint64(cg_cpu,"cpu.shares",val);
	if(err) {
		cg_error(err);
		exit(EXIT_FAILURE);
	}

	/* force lib to rewrite values */
	err = cgroup_modify_cgroup(burner);
	if(err) {
		cg_error(err);
		exit(EXIT_FAILURE);
	}
}


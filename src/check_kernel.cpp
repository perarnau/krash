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

#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <libcgroup.h>
#include <sched.h> // for affinity
#include <sys/wait.h>
#include "actions.hpp"
#include "cgroups.hpp"
#include "cpuinjector.hpp"
#include "errors.hpp"
#include "events.hpp"
#include "utils.hpp"

#define REPEATS 20

/* Macro to time functions */
#define DECL_EXP		\
struct timespec _ts[REPEATS], _tf[REPEATS]; \
int _err = 0;		\
double _t[REPEATS];

#define START_ONE(i,lbl)			\
_t[i] = 0;					\
_err = clock_gettime(CLOCK_REALTIME, &_ts[i]) ;	\
if(_err) goto lbl;

#define CHECK_LIMIT(i,max) ((_t[i] < max))

#define STOP_ONE(i,lbl)				\
_err = clock_gettime(CLOCK_REALTIME, &_tf[i]) ;	\
if(_err) goto lbl;

#define SAVE_R(i)					\
_t[i] = 1e3*(double)(_tf[i].tv_sec - _ts[i].tv_sec)	\
   + (double)((_tf[i].tv_nsec - _ts[i].tv_nsec) / 1e6) ;

#define MEAN(m)					\
for(i = 0; i < REPEATS; i++) { m += _t[i]; }	\
m = m / REPEATS;

#define CHECK_ERROR_XP ((_err))

/* Writes a value to a file and loop until it can read it
 * back. The write is repeated inside the loop.
 * Returns 0 if the test succeeds, 1 if the test failed, 2 if an error
 * occurred.
 */
#define MAX_WRITE_LOOP_LATENCY 1000
static int write_loop(std::fstream *f)
{
	int i;
	DECL_EXP
	double mean = 0;
	int val;
	std::string wr,rd;
	/* new value must difer from current value */
	f->seekg(0);
	*f >> val;
	val += 42;
	wr = itos(val);
	rd = "";
	std::cout << "Starting brute-force write to VFS. Averaging measurements over "
		<< REPEATS << " runs, each must take less than "
		<< MAX_WRITE_LOOP_LATENCY << " ms." << std::endl;
	for(i = 0; i < REPEATS; i++) {
		/* since we don't want to loop forever if the kernel is
		* REALLY bugged, we inspect loop time after each read.
		*/
		START_ONE(i,end_xp)
		while(rd != wr && CHECK_LIMIT(i,MAX_WRITE_LOOP_LATENCY)) {
			// seek to file start
			f->seekp(0);
			*f << wr;
			f->flush();
			// seek to start again
			f->seekg(0);
			*f >> rd;
			STOP_ONE(i,end_xp)
			SAVE_R(i)
		}
	}
	MEAN(mean)
end_xp:
	if(CHECK_ERROR_XP) return 2;
	std::cout << "Brute-write to VFS takes an average of " << mean << " ms." << std::endl;
	if(mean < MAX_WRITE_LOOP_LATENCY) /* one second is quick enough */
		return 0;
	else
		return 1;
}

/* Write a value to a cgroup file and wait until it can be read again
 * by io on an associated fstream.
 */
#define MAX_CGROUP_LOOP_LATENCY 1000
static int cgroup_loop(cgroups::Cgroup *cg, std::fstream *f)
{
	int i;
	DECL_EXP
	double mean = 0;
	u_int64_t val,rd;
	f->seekg(0);
	*f >> val;
	val += 42;
	rd = 0;
	std::cout << "Starting libcgroup write to VFS. Averaging measurements over "
		<< REPEATS << " runs, each must take less then "
		<< MAX_CGROUP_LOOP_LATENCY << " ms." << std::endl;
	for(i = 0; i < REPEATS; i++) {
		START_ONE(i,end_xp)
		while(rd != val && CHECK_LIMIT(i,MAX_WRITE_LOOP_LATENCY)) {
			cg->set_cpu_shares(val);
			// seek to start
			f->seekg(0);
			*f >> rd;
			STOP_ONE(i,end_xp)
			SAVE_R(i)
		}
	}
	MEAN(mean)
end_xp:
	if(CHECK_ERROR_XP) return 2;
	std::cout << "Libcgroup write to VFS takes an average of " << mean << " ms." << std::endl;
	if(mean < MAX_CGROUP_LOOP_LATENCY) /* one second is quick enough */
		return 0;
	else
		return 1;

}

/* This function compares direct writes on the VFS to
 * libcgroup implementation.
 * It both checks that the VFS is responding quickly and that
 * libcgroup is not too slow.
 * We consider one second to be quick enough for correct injection.
 */
static int vfs_timing()
{
	int err = 0;
	cgroups::Cgroup *cg;
	std::fstream f;
	std::string path;
	char *cpu_mount;
	/* create a testing group below the target */
	cg = new cgroups::Cgroup("test");
	err = cg->lib_attach(true);
	TEST_FOR_ERROR(err,error)
	/* find the cpu.shares file */
	path = cgroups::cpu_mountpoint + cg->path + "/cpu.shares";
	f.open(path.c_str());
	if(!f.is_open()) { err = 1; goto error_detach; }

	err = write_loop(&f);
	if(err) {
		if(err == 2)
			std::cerr << "Error during a test, exiting..." << std::endl;
		if(err == 1)
			std::cerr << "Test failed, exiting..." << std::endl;
		goto error;
	}

	err = cgroup_loop(cg,&f);
	if(err) {
		if(err == 2)
			std::cerr << "Error during a test, exiting..." << std::endl;
		if(err == 1)
			std::cerr << "Test failed, exiting..." << std::endl;
		goto error;
	}
error:
	f.close();
error_detach:
	cg->lib_detach();
error_delete:
	delete cg;
	return err;
}

static u_int64_t slow_function(u_int64_t max)
{
	u_int64_t i,j,k,l,m;
	u_int64_t a,b,c;
	a = 2000;
	b = 3000;
	c = 4000;
	for(i = 0; i < max; i++)
	for(j = 0; j < max; j++)
	for(k = 0; k < max; k++)
	{
		a = b % (c+1);
		b = a/(c+1);
		c = b/(a+1);
		c = c % (a+1);
		b = b % (c+1);
		a = a/(b+1) % (c+1);
	}
	std::cout << " " << a;
	return a;
}

/* This determines a for loop limit so that it takes
 * at least max ms.
 */
static int calibrate_cpu_loop(u_int64_t *ret,int max)
{
	int err = 0;
	int i;
	u_int64_t cm,tmp;
	u_int64_t r;
	cm = tmp = 100;
	DECL_EXP
	std::cout << "Calibrating a for loop to last at least "
		<< max << " ms." << std::endl;
	do {
		tmp = cm*1.5;
		if(tmp < cm) {
			std::cerr << "Overflow in loop limit: " << tmp
				<< " " << cm << std::endl;
			return 1;
		}
		cm = tmp;
		START_ONE(0,end_xp);
		r = slow_function(cm);
		STOP_ONE(0,end_xp);
		SAVE_R(0);
	} while(CHECK_LIMIT(0,max));
end_xp:
	std::cout << std::endl;
	if(CHECK_ERROR_XP) return 1;
	*ret = cm;
	std::cout << "Found a good loop lasting "
		<< _t[0] << " ms." << std::endl;
	return 0;
}

/* This action will launch a process runnig the cpu loop and register its timing
 * inside measure
 */
class CPULoopTimingAction : public Action {
	public:
		CPULoopTimingAction(std::string id, unsigned int time, double *measure, bool stop_loop, u_int64_t limit);

		int activate();


		double *m;
		ev::child *w;
		bool stop;
		struct timespec start,end;
		u_int64_t loop_limit;
};

CPULoopTimingAction::CPULoopTimingAction(std::string id, unsigned int time, double *measure, bool stop_loop, u_int64_t limit)
	: Action(id,time)
{
	this->m = measure;
	this->stop = stop_loop;
	this->loop_limit = limit;
}

void cpuloop_child_callback(ev::child &w, int revent)
{
	int err;
	std::cout << "Child Exited, process id " << w.rpid << std::endl;
	CPULoopTimingAction *a = (CPULoopTimingAction*) w.data;
	/* stop timing and memorize result */
	err = clock_gettime(CLOCK_REALTIME, &a->end);
	if(err) return;
	*a->m = 1e3*(double)(a->end.tv_sec - a->start.tv_sec)
		+ (double)((a->end.tv_nsec - a->start.tv_nsec) / 1e6);
	std::cout << "Child took "  << *a->m << " ms." << std::endl;
	if(a->stop)
		events::stop(0);
}

int CPULoopTimingAction::activate()
{
	int err = 0;
	u_int64_t i;
	pid_t p;
	std::cout << "Measuring CPULoop..." << std::endl;
	err = clock_gettime(CLOCK_REALTIME, &this->start);
	if(err) return 1;
	p = fork();
	if(p == -1) return 1;
	if(p == 0) {
		/* attach myself to cpu 0 */
		cpu_set_t mycpuset;
		CPU_ZERO(&mycpuset);
		CPU_SET(0,&mycpuset);
		sched_setaffinity(0,sizeof(cpu_set_t),&mycpuset);
		slow_function(loop_limit);
		exit(EXIT_SUCCESS);
	}
	/* register a child watcher */
	std::cout << "Watching process id " << p << std::endl;
	w = new ev::child(*events::loop);
	w->set<cpuloop_child_callback>(NULL);
	w->data = this;
	w->set(p,0);
	w->start();
	return 0;
}

/* This function validates a simple load injection profile. It helps ensure
 * that the kernel is doing his job.
 * There is a lot of thing to do for this check:
 *	- callibrate a for loop so that it takes at least 5 seconds.
 *	- install krash on the system.
 *	- time a process running this loop and measure its mean time to completion.
 *	- create a basic profile and launch injection on it.
 *	- launch the same process and  measure the MTC.
 *	- check that loaded process is loaded correctly.
 *	- cleanup the injection.
 */
#define CPU_LOAD 70
static int cpu_load_check()
{
	int err = 0;
	u_int64_t loop_limit = 0;
	double m_unload, m_load;
	double real_load;
	ActionsList *l;
	int nr_cpus;
	/* calibrate a loop */
	std::cout << "Further experiments will dump garbage, don't worry about it"
		<< std::endl;
	err = calibrate_cpu_loop(&loop_limit,5*1e3);
	TEST_FOR_ERROR(err,error_calibrate);
	/* create an action list with measurements of unloaded and loaded processes
	 * and a cpuload injection */
	/* a simple profile */
	l = new ActionsList();
	l->push(new CPUAction(0,0,0));
	l->push(new CPULoopTimingAction("unloaded",1,&m_unload,false,loop_limit));
	l->push(new CPUAction(20,0,CPU_LOAD));
	/* OPTIONAL CHECK: if the system contains more than one CPU, we load another
	 * one to ensure group scheduling is per cpu only.
	 */
	nr_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if(nr_cpus != -1 && nr_cpus > 1)
		l->push(new CPUAction(20,1,CPU_LOAD/2));

	l->push(new CPULoopTimingAction("loaded",21,&m_load,true,loop_limit));
	/* we need to initialize events */
	err = events::setup(*l);
	TEST_FOR_ERROR(err,error_events);
	/* now install the cgroups */
	err = cgroups::install();
	TEST_FOR_ERROR(err,error_cgroups);
	/* setup cpu */
	err = cpuinjector::setup(*l);
	TEST_FOR_ERROR(err,error_cpu);
	/* launch event loop */
	err = events::start();
	TEST_FOR_ERROR(err,error_loop);
	/* at this point m_unload and m_load contain MTCs */
	std::cout << "Without load, process took " << m_unload
		<< " ms to complete." << std::endl;
	std::cout << "With a " << CPU_LOAD << " load, process took " << m_load
		<< " ms to complete." << std::endl;
	/* compute limits of correct load, a 5% load difference is ok */
	real_load = 100.0 - ((m_unload/m_load)*100.0);
	if(real_load > CPU_LOAD-5 && real_load < CPU_LOAD+5) {
		std::cout << "CPU Load seems to work, a " << CPU_LOAD << "\% load induced a measured load of "
			<< real_load << " on a cpu-intensive process." << std::endl;
		err = 0;
	}
	else {
		std::cout << "CPU Load seems bugged, a " << CPU_LOAD << "\% load induced a measure load of "
			<< real_load << " on a cpu-intensive process." << std::endl;
		err = 1;
	}
error_loop:
error_cpu:
	cpuinjector::cleanup();
error_cgroups:
	cgroups::remove();
error_events:
	events::cleanup();
error_calibrate:
	return err;
}

int check_kernel()
{
	int err = 0;
	/* first verify that writing to kernel VFS is not too long */
	err = vfs_timing();
	TEST_FOR_ERROR(err,ret)
	/* next verify that the group scheduling is working */
	err = cpu_load_check();
ret:
	return err;
}

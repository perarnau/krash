#include "cpuinjector.hpp"
#include "utils.hpp"
#include <libcgroup.h>
#include <cstdlib>
#include <iostream>
#include <set>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE // need by sched.h
#endif
#include <sched.h> // for affinity


/* static variables to easier save some general info on groups */
static std::string cpu_cgroup_root;
static std::string cpuset_cgroup_root;
static std::string alltasks_groupname;
static std::string cgroups_basename;
static unsigned int alltasks_priority;
static std::set<std::pair<unsigned int, pid_t> > active_cpus;

void cpuinjector_configure(std::string cpu_cg_root,std::string cpuset_cg_root,std::string all_name,std::string cg_basename,unsigned int all_prio) {
	cpu_cgroup_root = cpu_cg_root;
	cpuset_cgroup_root = cpuset_cg_root;
	alltasks_groupname = all_name;
	cgroups_basename = cg_basename;
	alltasks_priority = all_prio;
}

int setup_system() {
	struct cgroup *alltasks;
	void *handle = NULL;
	pid_t pid;
	int err;

	/* init cgroup library */
	err = cgroup_init();
	if(err != 0)
	{
		std::cerr << "Init Failed: " << cgroup_strerror(err) << std::endl;
		exit(EXIT_FAILURE);
	}

	/* create the /alltasks group */
	std::string all_path = cpu_cgroup_root + alltasks_groupname;
	alltasks = cgroup_new_cgroup(all_path.c_str());
	if(!alltasks)
		return 1;

	cgroup_add_controller(alltasks,CPU_CGROUP_NAME);

	cgroup_create_cgroup_from_parent(alltasks,0);

	/* migrate all tasks, using task walking functions */
	char cpugroup[] = CPU_CGROUP_NAME;
	err = cgroup_get_task_begin(NULL,cpugroup,&handle,&pid);
	if(err)
	{
		std::cerr << "Error: " << cgroup_strerror(err) << std::endl;
		exit(EXIT_FAILURE);
	}
	do
	{
		cgroup_attach_task_pid(alltasks,pid);
		// TODO: skip getpid()
	}
	while((err = cgroup_get_task_next(&handle,&pid)) == 0);
	if(err != ECGEOF)
	{
		std::cerr << "Error: " << cgroup_strerror(err) << std::endl;
		exit(EXIT_FAILURE);
	}
	cgroup_get_task_end(&handle);
	return 0;
}

int setup_cpu(unsigned int cpuid) {
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
	if(cgroup_add_controller(burner,CPU_CGROUP_NAME) == NULL) {
		std::cerr << "Error: " << cgroup_strerror(err) << std::endl;
		return 1;
	}
	err = cgroup_create_cgroup_from_parent(burner,0);
	if(err) {
		std::cerr << "Error: " << cgroup_strerror(err) << std::endl;
		return 1;
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
		std::cerr << "Error: " << cgroup_strerror(err) << std::endl;
		// TODO: clean allocated structures and processes
		return 1;
	}

	/* register cpu as active */
	active_cpus.insert(std::pair<unsigned int, pid_t>(cpuid,burner_pid));
	return 0;
}

int apply_share(unsigned int cpuid, unsigned int share) {
	/* what we need to do
	 * - know the priority of the alltask group
	 * - compute the priority to apply to a given cpu group
	 * - apply it
	 */
}


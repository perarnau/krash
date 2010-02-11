#include "cpuinjector.hpp"
#include <libcgroup.h>
#include <cstdlib>
#include <iostream>

int setup_system(std::string cpu_cgroup_root, std::string alltasks_name) {
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
	alltasks_name = cpu_cgroup_root + alltasks_name;
	alltasks = cgroup_new_cgroup(alltasks_name.c_str());
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
	}
	while((err = cgroup_get_task_next(&handle,&pid)) == 0);
	if(err != ECGEOF)
	{
		std::cerr << "Error: " << cgroup_strerror(err) << std::endl;
		exit(EXIT_FAILURE);
	}
	cgroup_get_task_end(&handle);
}

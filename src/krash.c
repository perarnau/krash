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

 /*
 * This program is a first attempt to code the setup
 * code from KRASH in C.
 * The setup is supposed to do the following, having root priviledges:
 *	- migrate all tasks under root in a new /all_tasks CPU control group,
 *	- create a number of control groups depending on the number of
 *	CPUs to load,
 *	- set itself as a task under root group
 */

#include<stdio.h>
#include<stdlib.h>
#include<libcgroup.h>


int main(int argc, char **argv)
{
	char *all_tasks_name = "alltasks";
	struct cgroup *alltasks;
	void *handle = NULL;
	pid_t pid;
	int err;
	/*
	char *burners_prefix = "burner.";
	int loaded_cpus = 16;
	*/

	/* init cgroup library */
	err = cgroup_init();
	if(err != 0)
	{
		fprintf(stderr,"Init Failed: %s\n",cgroup_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* create the /alltasks group */
	alltasks = cgroup_new_cgroup(all_tasks_name);
	if(!alltasks)
		return 1;

	cgroup_add_controller(alltasks,"cpu");
	cgroup_add_controller(alltasks,"cpuset");

	cgroup_create_cgroup_from_parent(alltasks,0);

	/* migrate all tasks, using task walking functions */
	err = cgroup_get_task_begin(NULL,"cpu",&handle,&pid);
	if(err)
	{
		fprintf(stderr,"Error: %s\n",cgroup_strerror(err));
		exit(EXIT_FAILURE);
	}
	do
	{
		cgroup_attach_task_pid(alltasks,pid);
	}
	while((err = cgroup_get_task_next(&handle,&pid)) == 0);
	if(err != ECGEOF)
	{
		fprintf(stderr,"Error: %s\n",cgroup_strerror(err));
		exit(EXIT_FAILURE);
	}
	cgroup_get_task_end(&handle);

	return 0;
}

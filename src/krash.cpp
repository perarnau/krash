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
#include<getopt.h>
#include<libcgroup.h>

#include "config.h"
#include "actions.hpp"
#include "profile-parser-driver.hpp"

/* Arguments parsing variables */
static int ask_help = 0;
static int verbose = 0;
static char* profile = NULL;
static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "verbose", no_argument, &verbose, 1 },
	{ "profile", required_argument, NULL, 'p' },
	{ 0,0,0,0 },
};

static const char short_opts[] = "hvp:";

/* Setup code for krash */
int setup()
{
	char all_tasks_name[] = "alltasks";
	struct cgroup *alltasks;
	void *handle = NULL;
	pid_t pid;
	int err;

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
	char cpugroup[] = "cpu";
	err = cgroup_get_task_begin(NULL,cpugroup,&handle,&pid);
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
}

int main(int argc, char **argv)
{
	int c,err;
	int option_index = 0;

	while(1)
	{
		c = getopt_long(argc, argv, short_opts,long_options, &option_index);
		if(c == -1)
			break;

		switch(c)
		{
			case 'v':
			case 'h':
			case 0:
				break;
			case 'p':
				profile = optarg;
				break;
			case '?':
			default:
				exit(1);
		}
	}
	//setup();
	ActionList *l = new ActionList();
	l->add_action(new CPUAction(std::string("cpu0"),0,0,50));
	l->add_action(new CPUAction(std::string("cpu1"),0,1,50));
	l->add_action(new CPUAction(std::string("cpu0"),10,0,50));
	l->add_action(new CPUAction(std::string("cpu0"),100,0,50));
	l->add_action(new CPUAction(std::string("cpu2"),100,2,50));
	l->add_action(new CPUAction(std::string("cpu0"),20,0,50));
	l->add_action(new CPUAction(std::string("cpu0"),50,0,50));
	l->add_action(new CPUAction(std::string("cpu0"),40,0,50));
	l->start();
	return 0;
}

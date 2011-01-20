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
#include "cgroups.hpp"
#include "errors.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/types.h>

namespace cgroups {

// STANDARD NAMES FOR CGROUPS
static char CPU_CGROUP_NAME[] = "cpu";
static char CPU_SHARES[] = "cpu.shares";

std::string target_path = "/";
std::string alltasks_groupname = "alltasks";
std::string cpu_mountpoint;
Cgroup *target = NULL;
Cgroup *All = NULL;

int Cgroup::lib_attach(bool create)
{
	int err;
	struct cgroup *cg = NULL;
	struct cgroup_controller *cgc = NULL;

	cg = cgroup_new_cgroup(this->path.c_str());
	TEST_FOR_ERROR_P(cg,err,error);

	/* to gather data on it, we must add the cpu controller and
	 * its cpu.shares control file */
	cgc = cgroup_add_controller(cg,CPU_CGROUP_NAME);
	TEST_FOR_ERROR_P(cgc,err,error_free);

	/* 1024 is default and a good value */
	err = cgroup_add_value_uint64(cgc,CPU_SHARES,1024);
	TEST_FOR_ERROR(err,error_free);

	if(create)
	{
		/* create the group on the filesystem, populating the cg
		 * with current values */
		err = cgroup_create_cgroup(cg,0);
		TEST_FOR_ERROR(err,error_free);
	}
	this->cg = cg;
	return 0;
error_free:
	cgroup_free(&cg);
error:
	return err;
}

int Cgroup::lib_detach()
{
	int err;
	int ret = 0;
	/** this function deletes the kernel cgroup and
	 * tries to move all tasks to the parent.
	 * It fails regularly if one process was killed just before
	 * it is called.
	 * So we ask the function to ignore migration errors.
	 * If it still fails, we retry if the error is Device busy.
	 */
	do {
		err = cgroup_delete_cgroup(this->cg,1);
	} while(err == ECGOTHER && errno == EBUSY);
	SAVE_RET(err,ret);
	cgroup_free(&this->cg);
	this->cg = NULL;
	return ret;
}


int Cgroup::move_pids(Cgroup target)
{
	int err;
	void *handle = NULL;
	pid_t pid;
	/* migrate all tasks, using task walking functions */
	// libcgroup is stupid so I need to pass it a char* and I
	// only have a const char*: lets do stupid things !
	err = cgroup_get_task_begin(&this->path[0],CPU_CGROUP_NAME,&handle,&pid);
	if(err == ECGEOF)
		goto end; // TODO: maybe this is bad, at least one task should be moved, no ?
	TEST_FOR_ERROR(err,error);

	do {
		/* WE MUST SKIP ERRORS HERE, NOT ALL TASKS CAN BE MOVED IF
		 * OUR ROOT IS THE REAL ROOT */
		cgroup_attach_task_pid(this->cg,pid);
	}
	while((err = cgroup_get_task_next(&handle,&pid)) == 0);
	TEST_FOR_ERROR_IGNORE(err,ECGEOF,error);

end:
	cgroup_get_task_end(&handle);
	return 0;
error:
	return err;
}

int Cgroup::attach(pid_t p)
{
	int err;
	/* attach pid to the burner group */
	err = cgroup_attach_task_pid(this->cg,p);
	TEST_FOR_ERROR(err,error);
	return 0;
error:
	return err;
}

int Cgroup::get_cpu_shares(u_int64_t *shares)
{
	int err;
	struct cgroup_controller *cgc = NULL;
	TEST_FOR_ERROR_P(shares,err,error);

	/* retrieve the cpu cgroup for all and get the cpu.shares value */
	cgc = cgroup_get_controller(this->cg,CPU_CGROUP_NAME);
	TEST_FOR_ERROR_P(cgc,err,error);

	err = cgroup_get_value_uint64(cgc,CPU_SHARES,shares);
	TEST_FOR_ERROR(err,error);
	return 0;
error:
	return err;
}

int Cgroup::set_cpu_shares(u_int64_t val)
{
	int err;
	struct cgroup_controller *cgc = NULL;
	/* get the cpu controller */
	cgc = cgroup_get_controller(this->cg,CPU_CGROUP_NAME);
	TEST_FOR_ERROR_P(cgc,err,error);

	/* set the new value */
	err = cgroup_set_value_uint64(cgc,CPU_SHARES,val);
	TEST_FOR_ERROR(err,error);

	/* force lib to rewrite values */
	err = cgroup_modify_cgroup(this->cg);
	TEST_FOR_ERROR(err,error);
	return 0;
error:
	return err;
}

int init()
{
	int err;
	char *mnt;

	/* init cgroup library */
	err = cgroup_init();
	TEST_FOR_ERROR(err,error);

	/* ask for the cpu subsystem mountpoint */
	err = cgroup_get_subsys_mount_point(CPU_CGROUP_NAME,&mnt);
	TEST_FOR_ERROR(err,error);
	cpu_mountpoint = mnt;
	free(mnt);

	/* create the target group */
	target = new Cgroup("");
	err = target->lib_attach(false);
	TEST_FOR_ERROR(err,error_target);

	return 0;
error_target:
	target->lib_detach();
	delete target;
error:
	target = NULL;
	return err;
}

int install()
{
	int err = 0;

	/* create the all group */
	All = new Cgroup(alltasks_groupname);
	err = All->lib_attach(true);
	TEST_FOR_ERROR(err,error);

	/** Move tasks to all */
	err = target->move_pids(*All);
	TEST_FOR_ERROR(err,error_all);

	return 0;
error_all:
	All->lib_detach();
error:
	delete All;
	All = NULL;
	return err;
}

int remove()
{
	int err;
	int ret = 0;
	/* we don't need to move back the tasks, libcg
	 * does it for us.*/

	/* delete all */
	if(All != NULL) {
		err = All->lib_detach();
		SAVE_RET(err,ret);
		delete All;
	}
	return ret;
}

int destroy()
{
	int err;
	int ret = 0;
	/* delete target */
	if(target != NULL) {
		err = target->lib_detach();
		SAVE_RET(err,ret);
		delete target;
	}

	return ret;
}

} // end of cgroups namespace

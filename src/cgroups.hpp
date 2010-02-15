#ifndef CGROUPS_HPP
#define CGROUPS_HPP 1

#include <libcgroup.h>
#include <map>
#include <string>

class Cgroup {
	public:
		Cgroup(std::string name);

		int create();

		int add_controller(std::string controller);

		int attach_pid(pid_t pid);

		int set_value_uint64(std::string controller_name,uint64_t value);
		uint64_t get_value_uint64(std::string controller_name);

		std::string name;
	private:
		struct cgroup* self;
		std::map<std::string, struct cgroup_controller *> controllers;

}


#endif // ! CGROUPS_HPP

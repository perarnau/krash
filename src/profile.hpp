#ifndef PROFILE_HPP
#define PROFILE_HPP 1

#include "actions.hpp"
#include <string>

class Profile {
	public:
		/** basic contructor, doesn't do anything */
		Profile() {
			list = new ActionsList;
			cpu_cg_root = "/";
			all_cg_name = "all";
			burner_cg_basename = "krash";
		}

		/** a list of all actions in the profile */
		ActionsList *list;

		/** the path from the cpu cgroup controller mount point
		 * to the group of tasks to load.
		 * Usually equals to /
		 */
		std::string cpu_cg_root;

		/** the name of the control group where we'll move all loaded tasks
		 */
		std::string all_cg_name;

		/** the basename of each control group needed by our burners */
		std::string burner_cg_basename;
};


#endif // ! PROFILE_HPP

#ifndef PROFILE_HPP
#define PROFILE_HPP 1

#include "actions.hpp"
#include "components.hpp"
#include <string>
#include <list>

class Profile {
	public:
		/** basic contructor, doesn't do anything */
		Profile() {
			actions = new ActionsList();
			components = new Components();
		}

		/** a list of all actions in the profile */
		ActionsList *actions;

		/** a list of all components initialized in the profile */
		Components *components;
};


#endif // ! PROFILE_HPP

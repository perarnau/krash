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
#ifndef PROFILE_HPP
#define PROFILE_HPP 1

#include "actions.hpp"
#include <string>
#include <list>

class Profile {
	public:
		/** basic contructor, doesn't do anything */
		Profile() {
			actions = new ActionsList();
			inject_cpu = false;
		}

		/** a list of all actions in the profile */
		ActionsList *actions;

		/** is cpu injection active ? */
		bool inject_cpu;
};


#endif // ! PROFILE_HPP

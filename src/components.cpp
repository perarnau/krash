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
#include "components.hpp"
#include "cpuinjector.hpp"
#include "events.hpp"
#include <iostream>

int Components::setup(ActionsList& list) {
	int err = 0;
	if(cpu) {
		err = cpuinjector::setup(list);
	}
	err = err || events::setup(list);
	return err;
}

int Components::cleanup() {
	int err =  0;
	if(cpu) {
		err = cpuinjector::cleanup();
	}
	err = err || events::cleanup();
	return err;
}

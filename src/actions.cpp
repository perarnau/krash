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

#include "actions.hpp"
#include "utils.hpp"
#include "cpuinjector.hpp"
/** for debug purpose only */
#include<iostream>

/** for time drifting code */
#include<map>

/* class Action */
Action::Action(std::string id, unsigned int time) {
	this->id = id;
	this->time = time;
}

/* class CPUAction */
CPUAction::CPUAction(std::string id, unsigned int time, unsigned int cpu, unsigned int load) : Action(id,time) {
	this->cpu = cpu;
	this->load = load;
}

CPUAction::CPUAction(unsigned int time, unsigned int cpu, unsigned int load) : Action(std::string("cpu"),time) {
	this->cpu = cpu;
	this->load = load;
	this->id += itos(cpu);
}

void CPUAction::activate() {
	std::cout << "Applying share " << this->load << " on cpu " << this->cpu << " asked for time " << this->time << std::endl;
	MainCPUInjector->apply_share(get_cpu(),get_load());
}



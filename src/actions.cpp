#include "actions.hpp"
#include "utils.hpp"

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

inline unsigned int CPUAction::get_cpu() {
	return this->cpu;
}

inline unsigned int CPUAction::get_load() {
	return this->load;
}

void CPUAction::activate() {
	std::cout << "Activated: my cpu: " << get_cpu() << " my id: " << get_id() << std::endl;
}



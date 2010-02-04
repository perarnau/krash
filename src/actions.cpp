#include "actions.hpp"

/** for debug purpose only */
#include<iostream>

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

inline unsigned int CPUAction::get_cpu() {
	return this->cpu;
}

inline unsigned int CPUAction::get_load() {
	return this->load;
}

void CPUAction::activate() {
	std::cout << "Activated: my cpu: " << get_cpu() << " my id: " << get_id() << std::endl;
}

/** Class ActionList */
ActionList::ActionList() {

}

void ActionList::add_action(Action *a) {
	list.push(a);
}

void ActionList::start() {
	Action *a;
	while(!list.empty()) {
		a = list.top();
		list.pop();
		std::cout << "ActionList: action: " << a->get_time() << ", " <<a->get_id() << std::endl;
	}
}

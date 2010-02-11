#include "profile-parser-driver.hpp"
#include <iostream>

extern int yyparse(ActionsList* list);

ParserDriver::ParserDriver(std::string file) {
	this->filename = file;
	this->list = new ActionsList();
}

int ParserDriver::parse() {
	return yyparse(this->list);
}

ActionsList* ParserDriver::get_actions() {
	return this->list;
}

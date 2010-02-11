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

#include "profile-parser-driver.hpp"
#include <iostream>

extern int yyparse(ActionsList* list);

ParserDriver::ParserDriver(std::string file) {
	this->filename = file;
	this->list = new ActionsList();
}

int ParserDriver::parse() {
	scan_begin();
	return yyparse(this->list);
	scan_end();
}

ActionsList* ParserDriver::get_actions() {
	return this->list;
}

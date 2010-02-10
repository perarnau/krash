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

#ifndef PROFILE_PARSER_DRIVER_HPP
#define PROFILE_PARSER_DRIVER_HPP 1

#include <string>
#include "actions.hpp"

class ParserDriver {
	public:
		ParserDriver(std::string file);

		int parse();

		ActionsList* get_actions();

		void scan_begin();
		void scan_end();
	private:
		std::string filename;
		ActionsList *list;
};

#endif //!PROFILE_PARSER_DRIVER_HPP

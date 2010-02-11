/* This file is part of KRASH.
*
*   KRASH is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   KRASH is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with KRASH.  If not, see <http://www.gnu.org/licenses/>.
*
*   Copyright Swann Perarnau, 2008
*/
/** Bison C++ interface, see the bison doc for info */
%skeleton "lalr1.cc"
%defines
%define parser_class_name "ProfileParser"
/*%define namespace "krash::parser"*/

%code requires {
#include <string>
#include "actions.hpp"
class ProfileParserDriver;
}

%parse-param { ProfileParserDriver& driver }
%lex-param { ProfileParserDriver& driver } 

%locations
%initial-action {
	//initialize the initial location
	@$.begin.filename = @$.end.filename = &driver.file;
}

%debug
%error-verbose

// Symbols
%union {
	Action *a;
}

%code {
#include "profile-parser-driver.hpp"
}

%token TOKMOUNT TOKMODE TOKNAME TOKCPU TOKPROFILE EQUAL TIME NUMBER WORD PATH OBRACK CBRACK;
%%
config:
	cpu_config
	;

cpu_config:
	TOKCPU OBRACK name_assign mount_assign profile CBRACK
	;

name_assign:
	TOKNAME EQUAL WORD
	;

mount_assign:
	TOKMOUNT EQUAL WORD 
	;

profile:
	TOKPROFILE OBRACK cpuid_profiles CBRACK
	;

cpuid_profiles:
	| cpuid_profiles cpuid_profile
	;

cpuid_profile:
	NUMBER  OBRACK event_list CBRACK
	;

event_list:
	| event_list event
	;

event:
	TIME NUMBER 
	;
%%
void yy::ProfileParser::error(const yy::ProfileParser::location_type &l, const std::string& m) {
	driver.error(l,m);
}

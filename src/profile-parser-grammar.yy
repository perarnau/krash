/* 
 * This file is part of KRASH.
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

%{
#include <string>
#include <iostream>
#include "profile.hpp"
#include "actions.hpp"

extern void yyerror(Profile p, std::string msg) {
	std::cerr << msg << std::endl;
}

int yylex(void);

/* variables for actions */
int cpu;

%}

%parse-param { Profile p }

%error-verbose

%token TOKCPU TOKPROFILE
%token NUMBER
%token OBRACK CBRACK;
%%
config:
	cpu_config
	;

cpu_config:
	TOKCPU OBRACK profile CBRACK
	;

profile:
	TOKPROFILE OBRACK cpuid_profiles CBRACK
	;

cpuid_profiles:
	| cpuid_profiles cpuid_profile
	;

cpuid_profile:
	NUMBER { cpu = $1; } OBRACK event_list CBRACK
	;

event_list:
	| event_list event
	;

event:
	NUMBER NUMBER
	{ CPUAction *a = new CPUAction($1,cpu,$2); p.list->push(a); }

	;
%%


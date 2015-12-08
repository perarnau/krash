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

%{
#include <iostream>
#include <string>
#include <cstdlib>
#include "profile.hpp"
#include "profile-parser-grammar.hh"
#include "profile-parser-driver.hpp"
	/* Work around an incompatibility in flex (at least versions
	   2.5.31 through 2.5.33): it generates code that does
	   not conform to C89.  See Debian bug 333231
	   <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.  */
#undef yywrap
#define yywrap() 1

%}

%option noyywrap nounput batch

%%
#[^\n]*	;
[ \t]+	;
\n	;
cpu			return TOKCPU;
profile			return TOKPROFILE;
kill			return TOKKILL;
cgroup_root		return CGROUP_ROOT;
all_name		return ALL_NAME;
burner_basename		return BURNER_BASENAME;
=			return EQUAL;
\{			return OBRACK;
\}			return CBRACK;
[0-9]+			yylval.num =std::atoi(yytext); return NUMBER;
[a-zA-z0-9_\-\/\.]+	yylval.id = new std::string(yytext);return ID;
%%
void ParserDriver::scan_begin() {
	if (filename == "-")
		yyin = stdin;
	else if (!(yyin = fopen (filename.c_str (), "r")))
	{
		std::cerr << "cannot open " << filename << std::endl;
		exit (1);
	}
}

void ParserDriver::scan_end() {
	fclose(yyin);
}

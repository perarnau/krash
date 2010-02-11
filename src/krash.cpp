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

/*
 * This program is a first attempt to code the setup
 * code from KRASH in C.
 * The setup is supposed to do the following, having root priviledges:
 *	- migrate all tasks under root in a new /all_tasks CPU control group,
 *	- create a number of control groups depending on the number of
 *	CPUs to load,
 *	- set itself as a task under root group
 */

#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>

#include "config.h"
#include "actions.hpp"
#include "events.hpp"
#include "profile-parser-driver.hpp"
#include "cpuinjector.hpp"

/* Arguments parsing variables */
static int ask_help = 0;
static int verbose = 0;
static char* profile = NULL;
static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "verbose", no_argument, &verbose, 1 },
	{ "profile", required_argument, NULL, 'p' },
	{ 0,0,0,0 },
};

static const char short_opts[] = "hvp:";


int main(int argc, char **argv)
{
	int c,err;
	int option_index = 0;

	while(1)
	{
		c = getopt_long(argc, argv, short_opts,long_options, &option_index);
		if(c == -1)
			break;

		switch(c)
		{
			case 'v':
			case 'h':
			case 0:
				break;
			case 'p':
				profile = optarg;
				break;
			case '?':
			default:
				exit(1);
		}
	}
	setup_system(std::string("/"),std::string("alltasks"));

	/* read the profile and parse it */
	ParserDriver *driver = new ParserDriver(std::string(profile));
	driver->parse();

	/* launch the event driver with the parsed actions */
	ActionsList *list = driver->get_actions();
	EventDriver e(*list);
	e.start(); // DO NOT RETURN
	return 0;
}

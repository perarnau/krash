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

#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include "config.h"
#include "actions.hpp"
#include "events.hpp"
#include "profile-parser-driver.hpp"
#include "profile.hpp"
#include "cpuinjector.hpp"

/* Arguments parsing variables */
static int ask_help = 0;
static char* profile = NULL;
static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "profile", required_argument, NULL, 'p' },
	{ 0,0,0,0 },
};

static const char short_opts[] = "hp:";

void print_usage() {
	std::cerr << "krash: a CPU load Injector" << std::endl;
	std::cerr << "usage: krash [options]" << std::endl;
	std::cerr << "options:" << std::endl;
	std::cerr << "-p/--profile: file to use as profile." << std::endl;
	std::cerr << "-h/--help:    print this usage." << std::endl;
}

int main(int argc, char **argv) {
	int c,err;
	int option_index = 0;
	std::string profile_file;
	ParserDriver *driver;
	Profile p;
	CPUInjector *inj = NULL;
	EventDriver *e = NULL;

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

	if(ask_help) {
		print_usage();
		exit(129);
	}

	/* read the profile and parse it */
	if(profile == NULL) {
		profile_file = "profile";
	}
	else {
		profile_file = profile;
	}

	driver = new ParserDriver(profile_file);
	err = driver->parse();
	if(err) {
		std::cerr << "Error while parsing " << profile_file << ",aborting..." << std::endl;
		goto error;
	}
	p = driver->profile;

	/** setup the system */
	inj = new CPUInjector(p.cpu_cg_root,p.all_cg_name,p.burner_cg_basename);
	MainCPUInjector = inj;
	err = inj->setup(*(p.list));
	if(err) {
		std::cerr << "Error during sytem setup, aborting..." << std::endl;
		goto error;
	}

	/* launch the event driver with the parsed actions */
	e = new EventDriver(*(p.list));
	e->start(); // Return only if eventdriver::stop is called
	// before exiting, cleanup the system:
	err = inj->cleanup();
	if(err) goto error_clean;
	delete inj;
	delete driver;
	exit(EXIT_SUCCESS);

error:
	if(inj) {
		err = inj->cleanup();
		if(err) {
error_clean:
			std::cerr << "Warning: errors occurred during cleanup, you should check for any left over processes or configuration." << std::endl;
		}
		delete inj;
	}
	delete driver;
	exit(EXIT_FAILURE);
}

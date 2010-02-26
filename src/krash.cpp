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
static int ask_version = 0;
static char* profile = NULL;
static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "version", no_argument, &ask_version, 1 },
	{ "profile", required_argument, NULL, 'p' },
	{ 0,0,0,0 },
};

static const char short_opts[] = "hVp:";

void print_usage() {
	std::cout << "krash: a CPU load Injector" << std::endl;
	std::cout << "usage: krash [options]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "-p/--profile: file to use as profile." << std::endl;
	std::cout << "-h/--help:    print this help." << std::endl;
	std::cout << "-V/--version: print krash version." << std::endl;
	std::cout << "Report bugs to: krash-commits@lists.ligforge.imag.fr" << std::endl;
	std::cout << "krash home page: <http://krash.ligforge.imag.fr/>" << std::endl;
}

void print_version() {
	std::cout << PACKAGE_NAME << " " << PACKAGE_VERSION << std::endl;
	std::cout << "Copyright (C) 2009-2010 Swann Perarnau"<< std::endl;
	std::cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << std::endl;
	std::cout << "This is free software: you are free to change and redistribute it."<< std::endl;
	std::cout << "There is NO WARRANTY, to the extent permitted by law."<< std::endl;
}

int main(int argc, char **argv) {
	int c,err;
	int option_index = 0;
	std::string profile_file;
	ParserDriver *driver = NULL;
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
			case 0:
				/* if option set a flag, nothing else to do */
				if(long_options[option_index].flag != 0)
					break;
				else
					exit(1);
			case 'V':
				ask_version = 1;
				break;
			case 'h':
				ask_help = 1;
				break;
			case 'p':
				profile = optarg;
				break;
			case '?':
			default:
				exit(1);
		}
	}

	if(ask_version) {
		print_version();
		exit(0);
	}

	if(ask_help || profile == NULL) {
		print_usage();
		exit(0);
	}

	/* read the profile and parse it */
	std::cout << "Parsing file " << profile_file << std::endl;
	profile_file = profile;
	driver = new ParserDriver(profile_file);
	err = driver->parse();
	if(err) {
		std::cerr << "Error while parsing " << profile_file << ",aborting..." << std::endl;
		goto error;
	}
	p = driver->profile;

	/** setup the system */
	std::cout << "Parsing finished, installing krash on system" << std::endl;
	inj = new CPUInjector(p.cpu_cg_root,p.all_cg_name,p.burner_cg_basename);
	MainCPUInjector = inj;
	err = inj->setup(*(p.list));
	if(err) {
		std::cerr << "Error during sytem setup, aborting..." << std::endl;
		goto error;
	}

	/* launch the event driver with the parsed actions */
	e = new EventDriver(*(p.list));
	std::cout << "Setup finished, starting krash" << std::endl;
	e->start(); // Return only if eventdriver::stop is called
	// before exiting, cleanup the system:
	std::cout << "Load injection finished, cleaning the system" << std::endl;
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

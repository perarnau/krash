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
#ifndef ERRORS_HPP
#define ERRORS_HPP 1

#include <iostream>
#include <libcgroup.h>
#include <errno.h>
/* ERROR POLICY:
 * The fact that libcgroup can fail in a _lot_ of ways means
 * we need to recover errors in a consistent way and let all krash
 * call the cleaner before exiting.
 * Each error checking should display a message on error.
 * All error codes should be reset to 1 after printing.
 * Every public method should return 0 or 1 (other values are for us).
 * Every internal method should clean what it created but not exit.
 * Each method should call cg_error before returning, this way we can trace the call stack
 */

/* this macro translates error code into understandable strings
 * We identify libcg errors according to their first possible value*/
#define translate_error(err) {								\
	if(err == 0) {									\
		std::cerr								\
		<< "Internal error: error handling on good condition, report this bug !";\
	} else if(err == 1) {								\
		std::cerr << "Error: a function returned an error code";		\
	} else if(err >= ECGROUPNOTCOMPILED) {						\
		std::cerr << "Libcg error: " << cgroup_strerror(err);			\
	} else {									\
		std::cerr << "System error: " << strerror(err);				\
	}										\
	std::cerr << ", func: " << __func__ << ", in file: " << __FILE__ << ":" << __LINE__<< std::endl;\
}

#define TEST_FOR_ERROR(err,label) {	\
	if(err) {			\
		translate_error(err);	\
		err = 1;		\
		goto label;		\
	}				\
}					\

#define TEST_FOR_ERROR_IGNORE(err,ignore,label) {	\
	if(err && err != ignore) {			\
		translate_error(err);			\
		err = 1;				\
		goto label;				\
	}						\
}							\

#define TEST_FOR_ERROR_P(p,err,label) {	\
	if(!p) {			\
		err = errno;		\
		translate_error(err);	\
		err = 1;		\
		goto label;		\
	}				\
}					\

// register something went wrong but don't exit
#define SAVE_RET(err,ret) if(err) { translate_error(err); ret = 1; }

#endif // ERRORS_HPP

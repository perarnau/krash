AM_CFLAGS= -g -O0 -Wall -Werror -ansi -pedantic
AM_YFLAGS = -d
bin_PROGRAMS = krash
krash_SOURCES = profile-parser-grammar.yy profile-parser-lex.ll \
		profile-parser-driver.cpp profile-parser-driver.hpp krash.cpp \
		actions.cpp actions.hpp events.cpp events.hpp utils.cpp utils.hpp \
		cpuinjector.cpp cpuinjector.hpp profile.hpp cgroups.hpp cgroups.cpp \
		check_kernel.cpp check_kernel.hpp
BUILT_SOURCES = profile-parser-grammar.h profile-parser-grammar.cc profile-parser-lex.cc
CLEANFILES = profile-parser-grammar.h profile-parser-grammar.cc profile-parser-lex.cc

AM_CFLAGS= -g -O2 -Wall -Werror -ansi -pedantic
AM_YFLAGS = -d
bin_PROGRAMS = krash 
krash_SOURCES = profile-parser-grammar.yy profile-parser-lex.ll profile-parser-driver.cpp krash.cpp actions.cpp
BUILT_SOURCES = profile-parser-grammar.h profile-parser-grammar.cc profile-parser-lex.cc
CLEANFILES = profile-parser-grammar.h profile-parser-grammar.cc profile-parser-lex.cc

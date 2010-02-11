#ifndef PROFILE_PARSER_DRIVER_H
#define PROFILE_PARSER_DRIVER_H 1

#include <string>

#include "actions.hpp"

/**Bison C++ interface
 * see the bison doc for info */

#include "profile-parser-grammar.h"

// Tell Flex the lexer's prototype ...
# define YY_DECL                                        \
  yy::ProfileParser::token_type                         \
  yylex (yy::ProfileParser::semantic_type* yylval,      \
	 yy::ProfileParser::location_type* yylloc,      \
	 ProfileParserDriver& driver)
// ... and declare it for the parser's sake.
YY_DECL;

/** Driver class for profile parser */
class ProfileParserDriver {
	public:
		ProfileParserDriver();

		// Handling the scanner.
		void scan_begin ();
		void scan_end ();
		bool trace_scanning;

		int parse(const std::string& f);
		std::string file;
		bool trace_parsing;

		// Error handling.
		void error (const yy::location& l, const std::string& m);
		void error (const std::string& m);
};

/** Contains a list of Actions
 *
 * This class handles a list of actions and the event loop code to activate KRASH componants when needed.
 */
class ActionList {
	public:
		/** basic constructor */
		ActionList();

		/** add an action to the list */
		void add_action(Action* a);

		/** start the action handling, launching the event loop **/
		void start();

		/** callback needed by the ev lib */
		void timer_callback(ev::timer &w, int revents);
	private:
		/** the list of actions, sorted by increasing times */
		std::priority_queue<Action*, std::vector<Action*>, gt_pointers > list;

		/** the event loop */
		struct ev::default_loop *loop;

		/** a timer watcher */
		ev::timer *watcher;

		/** the start time of the loop
		 * This is needed because ev handle timers
		 * in absolute time
		 */
		ev::tstamp start_time;
};

#endif // !PROFILE_PARSER_DRIVER

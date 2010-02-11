#ifndef PROFILE_PARSER_DRIVER_HPP
#define PROFILE_PARSER_DRIVER_HPP 1

#include <string>
#include "actions.hpp"

class ParserDriver {
	public:
		ParserDriver(std::string file);

		int parse();

		ActionsList* get_actions();

		void scan_begin();
		void scan_end();
	private:
		std::string filename;
		ActionsList *list;
};

#endif //!PROFILE_PARSER_DRIVER_HPP

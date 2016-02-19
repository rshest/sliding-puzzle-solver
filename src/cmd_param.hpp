#ifndef __CMD_PARAM__
#define __CMD_PARAM__

#include <string>
#include <vector>
#include <sstream>

class cmd_param {
	std::vector<std::string> _args;
public:
	cmd_param(int argc, char *argv[]) {
		for (int i = 0; i < argc; i++) {
			_args.push_back(std::string(argv[i]));
		}
	}

	template <typename T>
	bool get(const char* param_name, T& val) {
		std::string prefix = "--";
		prefix += param_name;
		for (auto& arg : _args) {
			if (arg.compare(0, prefix.size(), prefix) == 0)
			{
				std::istringstream is(arg);
				is.ignore(prefix.size() + 1, ' ');
				is >> val;
				return true;
			}
		}
		return false;
	}
};

#endif
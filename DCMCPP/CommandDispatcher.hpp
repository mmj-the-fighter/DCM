#ifndef _COMMAND_DISPATCHER_
#define _COMMAND_DISPATCHER_

#include <string>
#include <map>
#include <functional>
#include <vector>
#include <iostream>


class CommandDispatcher
{
	std::map<std::string, std::function<int(const std::vector<std::string>&)>> delegates;
public:
	void AddCommand(const std::string& cmd, std::function<int(const std::vector<std::string>&)> fn) {
		delegates[cmd] = fn;
	}


	bool RemoveCommand(const std::string& cmd) {
		auto it = delegates.find(cmd);
		if (it != delegates.end()) {
			delegates.erase(it);
			return true;
		}
		else {
			std::cout << "Command not found: " << cmd << std::endl;
			return false;
		}
	}

	bool ExecuteCommand(const std::vector<std::string>& cmdAndArgs) {
		if (cmdAndArgs.empty()) {
			std::cout << "Error: Command is empty." << std::endl;
			return false;
		}
		auto it = delegates.find(cmdAndArgs[0]);

		if (it != delegates.end()) {
			int r = it->second(cmdAndArgs);
			return (r==0);
		}
		else {
			std::cout << "Command not found: " << cmdAndArgs[0] << std::endl;
			return false;
		}
	}
};

#endif
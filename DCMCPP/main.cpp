#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include "CommandDispatcher.hpp"
#include "DebugConnectionManager.hpp"

bool ToInt(const std::string& str, int& num)
{
	try {
		num = std::stoi(str);
		return true;
	}
	catch (const std::invalid_argument& e) {
		return false;
	}
	catch (const std::out_of_range& e) {
		return false;
	}
	return false;
};

bool ToFloat(const std::string& str, float& num)
{
	try {
		num = std::stof(str);
		return true;
	}
	catch (const std::invalid_argument& e) {
		return false;
	}
	catch (const std::out_of_range& e) {
		return false;
	}
	return false;
};

class HealthController
{
	int health;
public:
	HealthController() :health(100) {};
	int UpdateHealth(const std::vector<std::string>& cmdAndArgs)
	{
		if (cmdAndArgs.size() < 2) {
			std::cout << "Error: Missing argument for update health." << std::endl;
			return 1;
		}
		int n;
		if (!ToInt(cmdAndArgs[1], n)) {
			std::cout << "Can't parse: " << cmdAndArgs[1] << " as int." << std::endl;
			return 2;
		}
		health += n;
		std::cout << "Health :" << health << std::endl;
		return 0;
	}

};



class RotationController
{
	float rotation_angle;
	float axis_x;
	float axis_y;
	float axis_z;


public:
	RotationController() :rotation_angle(0.0), axis_x(0.0), axis_y(1.0), axis_z(0.0) {}
	int UpdateRotation(const std::vector<std::string>& cmdAndArgs)
	{
		if (cmdAndArgs.size() < 2) {
			std::cout << "Error: Missing argument for update rotation." << std::endl;
			return 1;
		}
		float angle;
		if (!ToFloat(cmdAndArgs[1], angle)) {
			std::cout << "Can't parse: " << cmdAndArgs[1] << " as float." << std::endl;
			return 2;
		}
		rotation_angle = angle;
		std::cout << "Rotation Angle :" << rotation_angle << std::endl;
		return 0;
	}

	int UpdateAxis(const std::vector<std::string>& cmdAndArgs)
	{
		if (cmdAndArgs.size() < 4) {
			std::cout << "Error: Missing argument for update rotation." << std::endl;
			return 1;
		}
		float xa, ya, za;
		if (!ToFloat(cmdAndArgs[1], xa)) {
			std::cout << "Can't parse: " << cmdAndArgs[1] << " as float." << std::endl;
			return 2;
		}
		if (!ToFloat(cmdAndArgs[2], ya)) {
			std::cout << "Can't parse: " << cmdAndArgs[2] << " as float." << std::endl;
			return 3;
		}
		if (!ToFloat(cmdAndArgs[3], za)) {
			std::cout << "Can't parse: " << cmdAndArgs[3] << " as float." << std::endl;
			return 4;
		}
		axis_x = xa;
		axis_y = ya;
		axis_z = za;
		std::cout << "Rotation Axis :" << axis_x << " " << axis_y << " " << axis_z << std::endl;
		return 0;
	}

};

int print2(const std::vector<std::string>& cmdAndArgs)
{
	for (const auto& a: cmdAndArgs) {
		std::cout << a << std::endl;
	}
	return 0; 
}



int main()
{
	HealthController hc;
	RotationController rc;
	DebugConnectionManager dcm;

	std::shared_ptr<CommandDispatcher> pcd = std::make_shared<CommandDispatcher>();
	pcd->AddCommand("health", std::bind(&HealthController::UpdateHealth, &hc, std::placeholders::_1));
	pcd->AddCommand("rotation", std::bind(&RotationController::UpdateRotation, &rc, std::placeholders::_1));
	pcd->AddCommand("axis", std::bind(&RotationController::UpdateAxis, &rc, std::placeholders::_1));
	pcd->AddCommand("print", [](const std::vector<std::string>& cmdAndArgs)->int {std::cout << cmdAndArgs[1] << std::endl; return 0; });
	pcd->AddCommand("print2", print2);
	dcm.SetCommandDispatcher(pcd);

	bool eventLoop = true;
	while (eventLoop) {
		;
	}



	return 0;
}
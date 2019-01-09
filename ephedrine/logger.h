#pragma once
#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
class Logger
{
public:
	static void info(std::string message)
	{
		logger->info(message);
	}
	static std::shared_ptr<spdlog::logger> logger;
private:
	//static std::shared_ptr<spdlog::logger> logger;
};


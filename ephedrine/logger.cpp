#include "logger.h"

std::shared_ptr<spdlog::logger> Logger::logger = spdlog::stdout_color_mt("console");
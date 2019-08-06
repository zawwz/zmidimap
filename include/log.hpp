#ifndef LOG_HPP
#define LOG_HPP

#include <string>

#define DEFAULT_LOG_STATE true

extern bool log_on;

void log(const std::string& str);

#endif //LOG_HPP

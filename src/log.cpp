#include "log.hpp"

#include <stdio.h>

bool log_on=DEFAULT_LOG_STATE;

void log(const std::string& str)
{
  if(log_on)
    printf("%s", str.c_str());
}

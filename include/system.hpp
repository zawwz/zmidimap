#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "popen.h"

#define ANNOUNCE_COMMAND "aseqdump -p System:1"
#define LIST_COMMAND "aseqdump -l | tail -n +2 | cut -c10-42 | tr -s ' '"
#define LIST_EXTENDED_COMMAND "aseqdump -l | tail -n +2 | cut -c-42"


extern pid_t announce_thread_pid;

void device_check();

void announce_loop();

#endif //SYSTEM_HPP

#ifndef POPEN_HPP
#define POPEN_HPP

#include <stdio.h>
#include <signal.h>

FILE* popen2(const char* command, const char* type, pid_t* pid);

int pclose2(FILE* fp, pid_t pid);

#endif //POPEN_HPP

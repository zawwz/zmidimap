#include "system.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#include <string>
#include <vector>

#include <ztd/shell.hpp>

#include "device.hpp"

int announce_thread_pid = 0;

void kill_all()
{
  if(announce_thread_pid>0)
    kill(announce_thread_pid, SIGINT);
  for(auto it : device_list)
  {
    if(it->thread_pid>0)
      kill(it->thread_pid, SIGINT);
  }
}

void device_check()
{
  char* buff = NULL;
  size_t buff_size = 0;
  FILE *stream = popen(LIST_EXTENDED_COMMAND, "r");
  std::vector<std::pair<int,std::string>> ls_device;

  getline(&buff, &buff_size, stream); //discard the first line
  int i=0,j=0;
  int t;
  while ( getline(&buff, &buff_size, stream) > 0 ) //retrieve device lines
  {
    //port id get
    i=0;
    while(buff[i] == ' ')
      i++;
    j=i;
    while(buff[i] != ':')
      i++;
    t=stoi( std::string(buff+j, i-j) );
    //name get
    j=9;
    i=10;
    while(buff[i+1] != '\n')
      i++;
    while(buff[i-1] == ' ')
      i--;
    //insert element
    ls_device.push_back(std::make_pair(t, std::string(buff+j, i-j)));
  }
  pclose(stream);

  for ( auto dev : device_list ) // iterate devices
  {
    for ( auto ls = ls_device.begin() ; ls != ls_device.end() ; ls++ )
    {
      if( !dev->busy && dev->name == ls->second && ls->first > 0) //device detected
      {
        dev->client_id = ls->first;
        dev->start_loop();
      }
      if( dev->busy && dev->client_id == ls->first )
        ls->first = -1;
    }
  }

  if(buff != NULL)
    free(buff);
}

void announce_loop()
{
  char* buff = NULL;
  size_t buff_size = 0;
  FILE* stream = ztd::popen2(ANNOUNCE_COMMAND, "r", &announce_thread_pid);

  if (stream == NULL)
  {
    fprintf(stderr, "Error\n");
    return;
  }

  while ( getline(&buff, &buff_size, stream) > 0 )
  {
    if ( (strstr(buff, "Port start") != NULL) || (strstr(buff,"Port subscribed") != NULL) )
      device_check();
  }

  ztd::pclose2(stream, announce_thread_pid);

  if(buff != NULL)
    free(buff);
}

void filetime_loop(std::string const& filepath)
{
  struct stat attrib;
  stat(filepath.c_str(), &attrib);
  time_t last_time=attrib.st_ctime;
  while(true)
  {
    stat(filepath.c_str(), &attrib);
    if(attrib.st_ctime > last_time)
    {
      kill_all();
      last_time = attrib.st_ctime;
    }
    sleep(1);
  }
}

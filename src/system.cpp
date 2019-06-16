#include "system.hpp"

#include "device.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#define ANNOUNCE_COMMAND "aseqdump -p System:1"
#define LIST_COMMAND "aseqdump -l | tail -n +2 | cut -c10- | tr -s ' '"

void device_check()
{
  char* buff = NULL;
  size_t buff_size = 0;
  FILE *stream = popen(LIST_COMMAND, "r");
  std::string str;

  getline(&buff, &buff_size, stream); //discard the first line
  while ( getline(&buff, &buff_size, stream) > 0 ) //retrieve device lines
  {
    str += buff;
  }

  for ( auto it : device_list ) // iterate devices
  {
    if( !it->busy && str.find(it->name) != std::string::npos ) //device detected
    {
      printf("Device '%s' found\n", it->name.c_str());
      it->start_loop();
    }
  }

  if(buff != NULL)
    free(buff);
}

void announce_loop()
{
  char* buff = NULL;
  size_t buff_size = 0;
  FILE *stream = popen(ANNOUNCE_COMMAND,"r");

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

  if(buff != NULL)
    free(buff);
}

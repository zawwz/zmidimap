#include <signal.h>
#include <stdio.h>

#include "device.hpp"
#include "system.hpp"

#include "Filedat.hpp"
#include "options.hpp"

#include <cstring>

int main(int argc, char* argv[])
{

  signal(SIGCHLD, SIG_IGN); //signal that we aren't expecting returns from child processes

  OptionSet options;
  options.addOption(Option('f',"file",true));

  auto argvec = argVector(argc, argv);

  auto t = options.getOptions(argvec);
  std::vector<std::string> arg=t.first;
  if( !t.second )
  {
    fprintf(stderr, "Unexpected error\n");
    return 1;
  }

  if (arg.size() <= 0 || arg[0] == "")
  {
    fprintf(stderr, "No config file specified\n");
    return 2;
  }

  Filedat file(arg[0]);
  if (!file.readTest())
  {
    fprintf(stderr, "File '%s' unavailable\n", arg[0].c_str());
    return 10;
  }

  printf("Loading config file '%s'\n", arg[0].c_str());
  try
  {
    file.importFile();

    for(int i=0 ; i<file.chunk().listSize() ; i++)
    {
      Device *newDevice = new Device;
      newDevice->import_chunk(file[i]);
      device_list.push_back(newDevice);
      printf("Loaded %d commands for device '%s'\n", newDevice->nb_command, newDevice->name.c_str());
    }

    printf("Starting scan for devices\n");
    announce_loop();

    for(auto it : device_list)
    {
      delete it;
    }

  }
  catch (file_format_error& e)
  {
    printErrorIndex(e.data(), e.where(), e.what(), e.origin());
    return 11;
  }
  catch (chunk_format_error& e)
  {
    std::cerr << "Chunk Error: " << e.what() << std::endl ;
    return 11;
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 11;
  }

  return 0;
}

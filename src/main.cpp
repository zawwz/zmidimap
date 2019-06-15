#include <signal.h>
#include <stdio.h>

#include "device.hpp"
#include "system.hpp"

#include "Filedat.hpp"
#include "options.hpp"

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
  bool import_ok;
  try
  {
    import_ok = file.importFile();

    if(import_ok)
    {
      for(int i=0 ; i<file.chunk().listSize() ; i++)
      {
        Device *newDevice = new Device;
        newDevice->import_chunk(file.chunk()[i]);
        device_list.push_back(newDevice);
        printf("Added device '%s' with %d commands\n", newDevice->name.c_str(), newDevice->nb_command);
      }
    }
    else
    {
      fprintf(stderr, "Unknown config file error\n");
      return 2;
    }

    printf("Starting scan for devices\n");
    announce_loop();

    for(auto it : device_list)
    {
      delete it;
    }

  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 11;
  }

  return 0;
}

#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "device.hpp"
#include "system.hpp"
#include "log.hpp"
#include "help.h"

#include "Filedat.hpp"
#include "options.hpp"
#include "popen.h"


OptionSet options;

void help()
{
  printf("zmidimap [options] <midimap file>\n\nOptions:\n");
  options.printHelp(2, 25);
  printf("\nSee --file-format --command-tags --shell-format options for details on map file format\n");
}

void option_p(const std::string& port)
{
  std::string command="aseqdump -p '" + port + '\'';
  pid_t pid;
  FILE *stream = popen2(command.c_str(), "r", &pid);
  char* buff = NULL;
  size_t buff_size = 0;
  while (getline(&buff, &buff_size, stream) > 0)
  {
    if ( (strstr(buff, "Port unsubscribed") != NULL) ) // distonnected
    {
      kill(pid, SIGINT); // kill the process
    }
    else
      printf("%s", buff);
  }
  pclose2(stream, pid);
}

void cleanup()
{
  for(auto it : device_list)
    delete it;
}

void stop(int ret)
{
  kill(announce_thread_pid, SIGINT);
  exit(ret);
}

void inthandler(int dummy)
{
  stop(0);
}

int main(int argc, char* argv[])
{
  signal(SIGINT, inthandler);
  signal(SIGCHLD, SIG_IGN); //not expecting returns from child processes

  options.addOption(Option('h',"help",        false, "Display this help message"));
  options.addOption(Option("file-format",     false, "Display file format help"));
  options.addOption(Option("command-tags",    false, "Display for command tag help"));
  options.addOption(Option("shell-format",    false, "Display for shell format help"));
  options.addOption(Option('l',"list",        false, "List detected devices"));
  options.addOption(Option('L',"full-list",   false, "Print whole device list details"));
  options.addOption(Option('p',"port",        true,  "Connect to device and output to console", "device"));
  options.addOption(Option("no-log",          false, "Disable console logging"));
  // options.addOption(Option('i',"interactive", false, "Start in interactive mode"));

  auto argvec = argVector(argc, argv);

  auto t = options.getOptions(argvec);
  std::vector<std::string> arg=t.first;
  if( !t.second ) //invalid option
    return 1;

  //exit options
  Option* op=nullptr;
  op = options.findOption('h');
  if( op->activated )
  {
    help();
    return 0;
  }
  op = options.findOption("file-format");
  if( op->activated )
  {
    printf("%s\n", FILE_FORMAT);
    return 0;
  }
  op = options.findOption("command-tags");
  if( op->activated )
  {
    printf("%s\n", COMMAND_TAGS);
    return 0;
  }
  op = options.findOption("shell-format");
  if( op->activated )
  {
    printf("%s\n", SHELL_FORMAT);
    return 0;
  }
  op = options.findOption('h');
  if( op->activated )
  {
    help();
    return 0;
  }
  op = options.findOption('L');
  if( op->activated )
  {
    sh("aseqdump -l");
    return 0;
  }
  op = options.findOption('l');
  if( op->activated )
  {
    sh(LIST_COMMAND);
    return 0;
  }
  op = options.findOption('p');
  if( op->activated )
  {
    option_p(op->argument);
    return 0;
  }

  //behavioral options
  op = options.findOption("no-log");
  if( op->activated )
  {
    log_on=false;
  }

  //no argument: display help
  if (arg.size() <= 0 || arg[0] == "")
  {
    help();
    return 0;
  }

  Filedat file(arg[0]);
  if (!file.readTest())
  {
    fprintf(stderr, "File '%s' unavailable\n", arg[0].c_str());
    return 10;
  }

  //main processing
  try
  {
    log("Loading map file '" + arg[0] + "'\n");
    file.import_file();

    //create commands
    for(int i=0 ; i<file.chunk().listSize() ; i++)
    {
      Device *newDevice = new Device;
      newDevice->import_chunk(file[i]);
      device_list.push_back(newDevice);
      log("Loaded "+std::to_string(newDevice->nb_command)+" commands for device '"+newDevice->name+"'\n");
    }

    //main loop
    log("Starting scan for devices\n");
    announce_loop();
  }
  catch (format_error& e)
  {
    printFormatException(e);
    cleanup();
    stop(11);
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    stop(-1);
  }

  cleanup();
  return 0;
}

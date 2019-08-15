#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "device.hpp"
#include "system.hpp"
#include "log.hpp"
#include "help.h"

#include <ztd/filedat.hpp>
#include <ztd/options.hpp>
#include <ztd/shell.hpp>

ztd::option_set options;

void help()
{
  printf("zmidimap [options] <midimap file>\n\nOptions:\n");
  options.print_help(2, 25);
  printf("\nSee --file-format --command-tags --shell-format options for details on map file format\n");
}

void option_p(const std::string& port)
{
  std::string command="aseqdump -p '" + port + '\'';
  pid_t pid;
  FILE *stream = ztd::popen2(command.c_str(), "r", &pid);
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
  ztd::pclose2(stream, pid);
}

void cleanup()
{
  for(auto it : device_list)
    delete it;
}

void stop(int ret)
{
  if(announce_thread_pid>0)
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

  bool piped=false;
  if (!isatty(fileno(stdin)))
    piped = true;

  options.add(ztd::option('h',"help",        false, "Display this help message"));
  options.add(ztd::option("file-format",     false, "Display file format help"));
  options.add(ztd::option("command-tags",    false, "Display for command tag help"));
  options.add(ztd::option("shell-format",    false, "Display for shell format help"));
  options.add(ztd::option('l',"list",        false, "List detected devices"));
  options.add(ztd::option('L',"full-list",   false, "Print whole device list details"));
  options.add(ztd::option('p',"port",        true,  "Connect to device and output to console", "device"));
  options.add(ztd::option("no-log",          false, "Disable console logging"));
  // options.add(ztd::option('i',"interactive", false, "Start in interactive mode"));

  std::vector<std::string> arg;
  try
  {
    arg = options.process(argc, argv);
  }
  catch(ztd::option_error& err)
  {
    printf("Option error: %s\n", err.what());
    stop(1);
  }

  //exit options
  ztd::option* op=nullptr;
  op = options.find('h');
  if( op->activated )
  {
    help();
    stop(0);
  }
  op = options.find("file-format");
  if( op->activated )
  {
    printf("%s\n", FILE_FORMAT);
    stop(0);
  }
  op = options.find("command-tags");
  if( op->activated )
  {
    printf("%s\n", COMMAND_TAGS);
    stop(0);
  }
  op = options.find("shell-format");
  if( op->activated )
  {
    printf("%s\n", SHELL_FORMAT);
    stop(0);
  }
  op = options.find('h');
  if( op->activated )
  {
    help();
    stop(0);
  }
  op = options.find('L');
  if( op->activated )
  {
    sh("aseqdump -l");
    stop(0);
  }
  op = options.find('l');
  if( op->activated )
  {
    sh(LIST_COMMAND);
    stop(0);
  }
  op = options.find('p');
  if( op->activated )
  {
    option_p(op->argument);
    stop(0);
  }

  //behavioral options
  op = options.find("no-log");
  if( op->activated )
  {
    log_on=false;
  }

  //no argument: display help
  ztd::filedat file;
  bool no_arg=false;
  if (arg.size() <= 0 || arg[0] == "")
  {
    no_arg=true;
    if(!piped)
    {
      help();
      stop(0);
    }
  }
  else
  {
    file.setFilePath(arg[0]);
  }


  //main processing
  try
  {
    if(no_arg)
    {
      log("Loading map from stdin\n");
      file.import_stdin();
    }
    else
    {
      log("Loading map file '" + arg[0] + "'\n");
      file.import_file();
    }

    //create commands
    for(int i=0 ; i<file.data().listSize() ; i++)
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
  catch (ztd::format_error& e)
  {
    ztd::printFormatException(e);
    cleanup();
    stop(11);
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    stop(1);
  }

  cleanup();
  stop(0);
}

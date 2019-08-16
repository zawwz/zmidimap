#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "device.hpp"
#include "system.hpp"
#include "log.hpp"
#include "help.h"

#include "format.hpp"

#include <ztd/filedat.hpp>
#include <ztd/options.hpp>
#include <ztd/shell.hpp>

#define VERSION_STRING "v1.2"

ztd::option_set options;

void help()
{
  printf("zmidimap [options] <file>\n\nOptions:\n");
  options.print_help(4, 25);
  printf("\nSee --zfd-format --command-tags --shell-format options for details on map file format\n");
}

void version()
{
  printf("zmidimap %s\n", VERSION_STRING);
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

  options.add(ztd::option("\r  [Help]"));
  options.add(ztd::option('h',"help",         false, "Display this help message"));
  options.add(ztd::option('v',"version",      false, "Display version"));
  options.add(ztd::option("mim-format",       false, "Display mim file format help"));
  options.add(ztd::option("zfd-format",       false, "Display zfd file format help"));
  options.add(ztd::option("command-tags",     false, "Display for command tag help"));
  options.add(ztd::option("shell-format",     false, "Display for shell format help"));

  options.add(ztd::option("\r  [Format]"));
  options.add(ztd::option("no-log",           false, "Disable console logging"));

  options.add(ztd::option("\r  [Devices]"));
  options.add(ztd::option('l',"list",         false, "List detected devices"));
  options.add(ztd::option('L',"full-list",    false, "Print whole device list details"));
  options.add(ztd::option('p',"port",         true,  "Connect to device and output to console", "device"));
  options.add(ztd::option("\r  [Map file]"));
  options.add(ztd::option('o',"output",       true, "Output the resulting zfd map to file. - for stdout"));
  options.add(ztd::option('m',"mim",          false, "Read file in mim format"));
  options.add(ztd::option('z',"zfd",          false, "Read file in zfd format"));
  options.add(ztd::option("aligner",          true,  "String to use for aligning output map format. Default \\t", "string"));
  options.add(ztd::option("\rIf no file format is specified, the program will try to guess the format"));
  // options.add(ztd::option('i',"interactive", false, "Start in interactive mode"));

  std::vector<std::string> arg;
  try
  {
    arg = options.process(argc, argv);
  }
  catch(ztd::option_error& err)
  {
    printf("%s\n", err.what());
    stop(1);
  }

  ztd::option* op=nullptr;
  //exit options
  if( options.find('h')->activated )
  {
    help();
    stop(0);
  }
  if( options.find('v')->activated )
  {
    version();
    stop(0);
  }
  if( options.find("mim-format")->activated )
  {
    printf("%s\n", MIM_FORMAT);
    stop(0);
  }
  if( options.find("zfd-format")->activated )
  {
    printf("%s\n", ZFD_FORMAT);
    stop(0);
  }
  if( options.find("command-tags")->activated )
  {
    printf("%s\n", COMMAND_TAGS);
    stop(0);
  }
  if( options.find("shell-format")->activated )
  {
    printf("%s\n", SHELL_FORMAT);
    stop(0);
  }
  if( options.find('L')->activated )
  {
    sh("aseqdump -l");
    stop(0);
  }
  if( options.find('l')->activated )
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
  if( options.find("no-log")->activated )
  {
    log_on=false;
  }
  std::string aligner="\t";
  if(options.find("aligner")->activated)
  {
    aligner=options.find("aligner")->argument;
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
      if(options.find("zfd")->activated)
      {
        file.import_file();
      }
      else if(options.find("mim")->activated)
      {
        file.data() = mimtochk(file_strimport(arg[0]));
      }
      else
      {
        std::string filestr=file_strimport(arg[0]);
        if(is_mim(filestr))
        {
          file.data() = mimtochk(filestr);
        }
        else
        {
          file.import_file();
        }
      }
    }
    if(options.find('o')->activated)
    {
      if(options.find('o')->argument == "-") {
        std::cout << file.strval(aligner) << std::endl;
      }
      else {

      }
      return 0;
    }
    //create commands
    // potential parallel improvement
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

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

#define VERSION_STRING "v1.3.1a"

ztd::option_set options;

void help()
{
  printf("zmidimap [options] <file>\n\nOptions:\n");
  options.print_help(4, 25);
  printf("\n");
  printf("See --mim-format --zfd-format --command-tags --shell-format options for details on map file format\n");
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
  kill_all();
  exit(ret);
}

void inthandler(int dummy)
{
  stop(0);
}

void load_filedat(ztd::filedat& file, bool from_stdin, std::string const& path)
{
  if(from_stdin)
  {
    log("Loading map from stdin\n");
    file.setFilePath(path);
    std::string str=file_strimport(path);

    if(options.find("zfd")->activated)
    {
      file.data() = str;
    }
    else if(options.find("mim")->activated)
    {
      file.data() = mimtochk(str);
    }
    else
    {
      if(is_mim(str))
      {
        file.data() = mimtochk(str);
      }
      else
      {
        file.data() = str;
      }
    }

  }
  else
  {
    file.setFilePath(path);
    log("Loading map file '" + path + "'\n");
    if(options.find("zfd")->activated)
    {
      file.import_file();
    }
    else if(options.find("mim")->activated)
    {
      file.data() = mimtochk(file_strimport(path));
    }
    else
    {
      std::string filestr=file_strimport(path);
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
}

void load_commands(ztd::chunkdat const& data)
{
  clean_devices();
  for(int i=0 ; i<data.listSize() ; i++)
  {
    Device *newDevice = new Device;
    newDevice->import_chunk(data[i]);
    device_list.push_back(newDevice);
    log("Loaded "+std::to_string(newDevice->nb_command)+" commands for device '"+newDevice->name+"'\n");
  }
}

int main(int argc, char* argv[])
{
  signal(SIGINT, inthandler);
  signal(SIGCHLD, SIG_IGN); //not expecting returns from child processes

  bool autoreload=true;
  bool from_stdin=false;

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
  options.add(ztd::option('m',"mim",          false, "Read file in mim format"));
  options.add(ztd::option('z',"zfd",          false, "Read file in zfd format"));
  options.add(ztd::option('o',"output",       true, "Output the resulting zfd map to file. - for stdout", "file"));
  options.add(ztd::option("aligner",          true,  "String to use for aligning output map format. Default \\t", "string"));
  options.add(ztd::option("no-reload",        false, "Disable auto reloading when file changes are detected"));
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
    if(err.type() == ztd::option_error::unknown_option)
      help();
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
    ztd::shr("aseqdump -l");
    stop(0);
  }
  if( options.find('l')->activated )
  {
    ztd::shr(LIST_COMMAND);
    stop(0);
  }

  op = options.find('p');
  if( op->activated )
  {
    option_p(op->argument);
    stop(0);
  }

  if (options.find('o')->activated)
  {
    log_on=false;
  }

  //behavioral options
  if( options.find("no-reload")->activated )
  {
    autoreload=false;
  }
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
  std::string filepath;
  if (arg.size() <= 0 || arg[0] == "")
  {
    help();
    stop(0);
  }
  else
  {
    filepath=arg[0];
  }
  if(filepath == "-")
  {
    filepath = "/dev/stdin";
    from_stdin = true;
  }

  //main processing
  try
  {
    //load
    load_filedat(file, from_stdin, filepath);
    //output
    if(options.find('o')->activated)
    {
      if(options.find('o')->argument == "-") {
        std::cout << file.strval(aligner) << std::endl;
      }
      else {
        file.setFilePath(options.find('o')->argument);
        file.export_file();
      }
      return 0;
    }
    //create commands
    load_commands(file.data());

    //main loop
    log("Starting scan for devices\n");
    if(autoreload)
      std::thread(filetime_loop, filepath).detach(); // start the killer thread
    announce_loop(); // loop until killed
    ztd::chunkdat bak_data = file.data();
    while(autoreload)
    {
      log("Reloading file\n");
      try
      {
        load_filedat(file, from_stdin, filepath);
        load_commands(file.data());
        bak_data = file.data();
      }
      catch (ztd::format_error& e)
      {
        ztd::printFormatException(e);
        log("Reloading old config\n");
        load_commands(bak_data);
      }
      catch (std::exception& e)
      {
        std::cerr << "Exception: " << e.what() << std::endl;
        log("Reloading old config\n");
        load_commands(bak_data);
      }
      announce_loop(); // loop until killed
    }
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

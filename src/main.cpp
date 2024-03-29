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

    if(options["zfd"])
    {
      file.data() = str;
    }
    else if(options["mim"])
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
    if(options["zfd"])
    {
      file.import_file();
    }
    else if(options["mim"])
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

  options.add(
    ztd::option("\r  [Help]"),
    ztd::option('h',"help",         false, "Display this help message"),
    ztd::option('v',"version",      false, "Display version"),
    ztd::option("mim-format",       false, "Display mim file format help"),
    ztd::option("zfd-format",       false, "Display zfd file format help"),
    ztd::option("command-tags",     false, "Display for command tag help"),
    ztd::option("shell-format",     false, "Display for shell format help"),
    ztd::option("\r  [Format]"),
    ztd::option("no-log",           false, "Disable console logging"),
    ztd::option("\r  [Devices]"),
    ztd::option('l',"list",         false, "List detected devices"),
    ztd::option('L',"full-list",    false, "Print whole device list details"),
    ztd::option('p',"port",         true,  "Connect to device and output to console", "device"),
    ztd::option("\r  [Map file]"),
    ztd::option('m',"mim",          false, "Read file in mim format"),
    ztd::option('z',"zfd",          false, "Read file in zfd format"),
    ztd::option('o',"output",       true,  "Output the resulting map to file", "file"),
    ztd::option("out-zfd",          false, "Output in zfd format"),
    ztd::option("aligner",          true,  "String to use for aligning output map format. Default \\t", "string"),
    ztd::option("no-reload",        false, "Disable auto reloading when file changes are detected")
  );

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

  //exit options
  if( options['h'] )
  {
    help();
    stop(0);
  }
  if( options['v'] )
  {
    version();
    stop(0);
  }
  if( options["mim-format"] )
  {
    printf("%s\n", MIM_FORMAT);
    stop(0);
  }
  if( options["zfd-format"] )
  {
    printf("%s\n", ZFD_FORMAT);
    stop(0);
  }
  if( options["command-tags"] )
  {
    printf("%s\n", COMMAND_TAGS);
    stop(0);
  }
  if( options["shell-format"] )
  {
    printf("%s\n", SHELL_FORMAT);
    stop(0);
  }
  if( options['L'] )
  {
    ztd::shr("aseqdump -l");
    stop(0);
  }
  if( options['l'] )
  {
    ztd::shr(LIST_COMMAND);
    stop(0);
  }

  if( options['p'] )
  {
    option_p(options['p']);
    stop(0);
  }

  if (options['o'])
  {
    log_on=false;
  }

  //behavioral options
  if( options["no-reload"] )
  {
    autoreload=false;
  }
  if( options["no-log"] )
  {
    log_on=false;
  }
  std::string aligner="\t";
  if(options["aligner"])
  {
    aligner=options["aligner"].argument;
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
    if(options['o'])
    {
      std::string ret;
      if(options["out-zfd"])
        ret=file.data().str(0, aligner);
      else
        ret=chktomim(file.data(), aligner);
      if(options['o'].argument == "-") {
        std::cout << ret << std::endl;
      }
      else {
        std::ofstream output(options['o']);
        if(!output)
        {
          std::cerr << "Cannot write to file '" + options['o'].argument + "'\n";
          return 1;
        }
        output << ret << std::endl ;
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

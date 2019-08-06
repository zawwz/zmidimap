#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <string>
#include <vector>
#include <thread>

#include "command.hpp"
#include "Filedat.hpp"

void sh(std::string const& string);

class Device
{
public:
  Device();
  virtual ~Device();

  bool start_loop();
  void run_signal(char* buff);

  bool import_chunk(Chunk const& ch);
  Chunk export_chunk();

  std::string name;
  int client_id;
  bool busy;

  uint32_t nb_command;
  std::vector<NoteCommand> noteCommands[128];
  std::vector<ControllerCommand> ctrlCommands[128];
  std::vector<PitchCommand> pitchCommands;
  std::vector<SystemCommand> sysCommands;
  std::vector<ConnectCommand> connectCommands;
  std::vector<DisconnectCommand> disconnectCommands;

  std::thread thread;
  pid_t thread_pid;
private:
  static void loop(Device* dev);
};

extern std::vector<Device*> device_list;

#endif //DEVICE_HPP

#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <string>
#include <vector>
#include <thread>

#include "command.hpp"

#include "Filedat.hpp"

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
  bool busy;

  uint32_t nb_command;
  std::vector<NoteCommand> noteCommands[128];
  std::vector<ControllerCommand> ctrlCommands[128];
  std::vector<PitchCommand> pitchCommands;
  // std::vector<Command> sysCommands;

  std::thread thread;
private:
  static void loop(Device* dev);
};

extern std::vector<Device*> device_list;

#endif //DEVICE_HPP
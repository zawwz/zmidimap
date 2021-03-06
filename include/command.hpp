#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <stdlib.h>

#include <string>

class NoteCommand
{
public:

  NoteCommand(int8_t ch, uint8_t l, uint8_t h, std::string const& sh);

  int8_t channel;
  uint8_t low;
  uint8_t high;
  std::string shell;
};

class ControllerCommand
{
public:

  ControllerCommand(int8_t ch, uint8_t l, uint8_t h, float ml, float mm, bool fl, std::string const& sh);

  int8_t channel;
  uint8_t min;
  uint8_t max;
  float mapMin;
  float mapMax;
  bool floating;
  std::string shell;
};

class PitchCommand
{
public:

  PitchCommand(uint8_t ch, int16_t l, int16_t h, float ml, float mh, bool fl, std::string const& sh);

  int8_t channel;
  int16_t min;
  int16_t max;
  float mapMin;
  float mapMax;
  bool floating;
  std::string shell;
};

class ConnectCommand
{
public:
  ConnectCommand(std::string const& sh);

  std::string shell;
};

class DisconnectCommand
{
public:
  DisconnectCommand(std::string const& sh);

  std::string shell;
};

class SystemCommand
{
public:

  SystemCommand(std::string const& sh);

  std::string shell;
};

#endif //COMMAND_HPP

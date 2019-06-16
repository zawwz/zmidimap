#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <stdlib.h>

#include <string>

class NoteCommand
{
public:

  NoteCommand(uint8_t i, int8_t ch, uint8_t l, uint8_t h, std::string sh);

  uint8_t id;
  int8_t channel;
  uint8_t low;
  uint8_t high;
  std::string shell;
};

class ControllerCommand
{
public:

  ControllerCommand(uint8_t i, int8_t ch, uint8_t l, uint8_t h, float ml, float mm, bool fl, std::string sh);

  uint8_t id;
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

  PitchCommand(uint8_t ch, int16_t l, int16_t h, float ml, float mh, bool fl, std::string sh);

  int8_t channel;
  int16_t min;
  int16_t max;
  float mapMin;
  float mapMax;
  bool floating;
  std::string shell;
};


#endif //COMMAND_HPP
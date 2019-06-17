#include "command.hpp"

NoteCommand::NoteCommand(int8_t ch, uint8_t l, uint8_t h, std::string const& sh)
{
  this->channel=ch;
  this->low=l;
  this->high=h;
  this->shell=sh;
}

ControllerCommand::ControllerCommand(int8_t ch, uint8_t l, uint8_t h, float ml, float mh, bool fl, std::string const& sh)
{
  this->channel=ch;
  this->min=l;
  this->max=h;
  this->mapMin=ml;
  this->mapMax=mh;
  this->floating=fl;
  this->shell=sh;
}

PitchCommand::PitchCommand(uint8_t ch, int16_t l, int16_t h, float ml, float mh, bool fl, std::string const& sh)
{
  this->channel=ch;
  this->min=l;
  this->max=h;
  this->mapMin=ml;
  this->mapMax=mh;
  this->floating=fl;
  this->shell=sh;
}

SystemCommand::SystemCommand(std::string const& sh)
{
  this->shell=sh;
}

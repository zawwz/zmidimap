#include "command.hpp"

NoteCommand::NoteCommand(uint8_t i, uint8_t ch, uint8_t l, uint8_t h, std::string sh)
{
  this->id=i;
  this->channel=ch;
  this->low=l;
  this->high=h;
  this->shell=sh;
}

ControllerCommand::ControllerCommand(uint8_t i, int8_t ch, uint8_t l, uint8_t h, float ml, float mh, bool fl, std::string sh)
{
  this->id=i;
  this->channel=ch;
  this->min=l;
  this->max=h;
  this->mapMin=ml;
  this->mapMax=mh;
  this->floating=fl;
  this->shell=sh;
}

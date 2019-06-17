#include "device.hpp"

#include "stringTools.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>

#define KILL_COMMAND_FH "kill -s INT $(ps -f | grep \"aseqdump -p "
#define KILL_COMMAND_SH "\" | grep -v grep | awk '{print $2}' | head -n1)"

std::vector<Device*> device_list;

static void sh(std::string const& string)
{
  system(string.c_str());
}

Device::Device()
{
  busy=false;
  nb_command=0;
}

Device::~Device()
{

}

bool Device::start_loop()
{
  if(this->busy)
    return false;
  this->busy = true;
  this->thread = std::thread(Device::loop, this);
  this->thread.detach();
  return true;
}

std::pair<int32_t,int32_t> importRange(Chunk const& ch, std::string const& tag, int32_t low, int32_t high)
{
  Chunk* pch=ch.subChunkPtr(tag);
  if(pch != nullptr)
  {
    std::string str=pch->strval();
    auto tpos=str.find(':');
    if (str=="*") //whole range
    {
      return std::make_pair(low,high);
    }
    else if(tpos == std::string::npos) //single value
    {
      low=stoi(str);
      high=low;
    }
    else //range
    {
      low=stoi(str.substr(0,tpos));
      tpos++;
      high=stoi(str.substr(tpos, str.size()-tpos));
    }
  }
  return std::make_pair(low, high);
}

std::pair<float,float> importRangeFloat(Chunk const& ch, std::string const& tag, float low, float high)
{
  Chunk* pch=ch.subChunkPtr(tag);
  if(pch != nullptr)
  {
    std::string str=pch->strval();
    auto tpos=str.find(':');
    if(tpos == std::string::npos)
    {
      //single value
      low=stof(str);
      high=low;
    }
    else
    {
      //range
      low=stof(str.substr(0,tpos));
      tpos++;
      high=stof(str.substr(tpos, str.size()-tpos));
    }
  }
  return std::make_pair(low, high);
}

bool importBool(Chunk const& ch, std::string const& tag, bool defbool)
{
  Chunk* pch=ch.subChunkPtr(tag);
  if(pch != nullptr)
  {
    std::string str=pch->strval();
    if( str == "true" )
      return true;
    else if( str == "false" )
      return false;
  }
  return defbool;
}

bool Device::import_chunk(Chunk const& ch)
{
  Chunk& cch = ch["commands"];
  this->name=ch["name"].strval();
  for(int i=0 ; i<cch.listSize() ; i++)
  {
    Chunk& tch=cch[i];
    std::string tstr=tch["type"].strval();
    if(tstr == "system") //type system
    {
      throw std::runtime_error("System commands not implemented yet");
    }
    else
    {
      int8_t channel;
      std::string shell;

      std::pair<int32_t,int32_t> intpair;
      std::pair<float,float> floatpair;

      //channel
      if(tch.subChunkPtr("channel") == nullptr || tch["channel"].strval()=="*")
        channel=-1;
      else
        channel=stoi(tch["channel"].strval());

      //shell
      shell=tch["shell"].strval();

      //type
      if(tstr == "note") //type note
      {
        uint8_t id_low=0;
        uint8_t id_high=127;
        uint8_t low=1;
        uint8_t high=127;
        std::string tt;

        //id
        intpair=importRange(tch, "id", id_low, id_high);
        id_low=intpair.first;
        id_high=intpair.second;
        // id=stoi(tch["id"].strval());

        //trigger
        intpair = importRange(tch, "trigger", low, high);
        low=intpair.first;
        high=intpair.second;

        for( uint8_t i=id_low ; i <= id_high ; i++ )
        {
          this->noteCommands[i].push_back(NoteCommand(channel,low,high,shell));
        }
        this->nb_command++;
      }
      else if(tstr == "controller") //type controller
      {
        uint8_t id_low=0;
        uint8_t id_high=127;
        uint8_t min=0;
        uint8_t max=127;
        float mapMin;
        float mapMax;
        bool floating=false;

        //id
        intpair=importRange(tch, "id", id_low, id_high);
        id_low=intpair.first;
        id_high=intpair.second;

        //range
        intpair = importRange(tch, "range", min, max);
        min=intpair.first;
        max=intpair.second;

        //remap
        floatpair = importRangeFloat(tch, "remap", min, max);
        mapMin=floatpair.first;
        mapMax=floatpair.second;

        //floating
        floating = importBool(tch, "float", floating);

        for( uint8_t i=id_low ; i <= id_high ; i++ )
        {
          this->ctrlCommands[i].push_back(ControllerCommand(channel,min,max,mapMin,mapMax,floating,shell));
        }
        this->nb_command++;
      }
      else if(tstr == "pitch") //type pitch bend
      {
        int16_t min=-8192;
        int16_t max=8191;
        float mapMin;
        float mapMax;
        bool floating=false;

        //range
        intpair = importRange(tch, "range", min, max);
        min=intpair.first;
        max=intpair.second;

        //remap
        floatpair = importRangeFloat(tch, "remap", min, max);
        mapMin=floatpair.first;
        mapMax=floatpair.second;

        //floating
        floating = importBool(tch, "float", floating);

        this->pitchCommands.push_back(PitchCommand(channel,min,max,mapMin,mapMax,floating,shell));
        this->nb_command++;
      }
      else
      {
        throw std::runtime_error("Command type " + tstr + " doesn't exist");
        return false;
      }
    }
  }
  return true;
}

void Device::run_signal(char* buff)
{
  if ( (strstr(buff, "Port unsubscribed") != NULL) ) // distonnected
  {
    std::string kill_command=KILL_COMMAND_FH + this->name + KILL_COMMAND_SH;
    system(kill_command.c_str()); // kill the process
    this->busy=false;
  }

  else if (index(buff, ':') != NULL) // MIDI command
  {
    if (strstr(buff, "System exclusive") != NULL)
    {
      printf("%s", buff);
      //do stuff
    }
    else
    {
      int t;
      char type;
      int8_t channel;
      int ctid;
      int16_t value;
      char* pos=NULL;
      bool note_off=false;

      //note off special case
      if (strstr(buff,"Note off") != NULL)
        note_off=true;

      // type read
      if (strstr(buff,"note") != NULL)
        type='n';
      else if (strstr(buff,"controller") != NULL)
        type='c';
      else if (strstr(buff,"Pitch") != NULL)
        type='p';
      else
      {
        throw std::runtime_error("Unknown MIDI signal:\n" + std::string(buff));
        return;
      }

      //channel read
      pos=index(buff, ',');
      t=1;
      while (*(pos-t-1) != ' ')
        t++;
      channel = std::stoi( std::string(pos-t, t) );
      pos+=2;

      //ctid read (only note and controller)
      if(type=='n' || type=='c')
      {
        while (*(pos) != ' ')
          pos++;
        pos++;
        t=1;
        while ( isNum(*(pos+t)) )
          t++;
        ctid = std::stoi( std::string(pos, t) );
        pos+=t+2;
      }

      //value read
      if(!note_off)
      {
        while (*(pos) != ' ')
          pos++;
        pos++;
        t=1;
        while ( isNum(*(pos+t)) )
          t++;
        value = std::stoi( std::string(pos, t));
      }
      else
        value=0;

      //processing
      if (type == 'n')
      {
        for( auto it : this->noteCommands[ctid] )
        {
          if( (it.channel == -1 || it.channel == channel) && it.low <= value && it.high >= value )
          {
            std::string command="id=" + std::to_string(ctid)
            + ";channel=" + std::to_string(channel)
            + ";velocity=" + std::to_string(value) + ";";
            command += it.shell;
            std::thread(sh, command).detach();
          }
        }
      }
      else if(type == 'c')
      {
        for( auto it : this->ctrlCommands[ctid] )
        {
          if( (it.channel == -1 || it.channel == channel) && it.min <=  value && it.max >= value )
          {
            //remapping of value
            float result;
            if(it.min == it.max) //zero case
              result = it.mapMin;
            else
              result=(value-it.min)*(it.mapMax-it.mapMin)/(it.max-it.min)+it.mapMin;

            //command execution
            std::string command="id=" + std::to_string(ctid)
            + ";channel=" + std::to_string(channel)
            + ";rawvalue=" + std::to_string(value)
            + ";value=";
            if(it.floating)
              command += std::to_string(result);
            else
              command += std::to_string((long int) result);
            command += ";" + it.shell;
            std::thread(sh, command).detach();
          }
        }
      }
      else if(type == 'p')
      {
        for( auto it : this->pitchCommands )
        {
          if( (it.channel == -1 || it.channel == channel) && it.min <= value && it.max >= value )
          {
            //remapping of value
            float result;
            if(it.min == it.max) //zero case
              result = it.mapMin;
            else
              result=(value-it.min)*(it.mapMax-it.mapMin)/(it.max-it.min)+it.mapMin;

            //command execution
            std::string command=";channel=" + std::to_string(channel)
            + ";rawvalue=" + std::to_string(value)
            + ";value=";
            if(it.floating)
              command += std::to_string(result);
            else
              command += std::to_string((long int) result);
            command += ";" + it.shell;
            std::thread(sh, command).detach();
          }
        }
      } // if type

    } //if system exclusive

  } //while

}

void Device::loop(Device* dev)
{
  std::string command = "aseqdump -p '" + dev->name + '\'';
  FILE *stream = popen(command.c_str(), "r");
  char* buff = NULL;
  size_t buff_size = 0;

  while (getline(&buff, &buff_size, stream) > 0)
  {
    dev->run_signal(buff);
  }

  printf("Device '%s' disconnected\n", dev->name.c_str());

  pclose(stream);
  free(buff);
}

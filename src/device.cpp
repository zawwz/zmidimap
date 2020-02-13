#include "device.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <iostream>

#include <ztd/shell.hpp>

#include "log.hpp"

std::vector<Device*> device_list;

static std::string dequote(const std::string& in)
{
  std::string ret;
  const char quote = in[0];
  if(quote != '\'' && quote !='\"')
    return in;
  unsigned int i=1;
  while(i<in.size())
  {
    if(in[i] == quote) {
      break;
    }
    else if(i+1 < in.size() && in[i] == '\\' && in[i+1] == quote) {
      i+=2;
      ret += quote;
    }
    else{
      ret += in[i];
      i++;
    }
  }
  return ret;
}

static bool _isNum(const char& a)
{
  return (a>='0' && a<='9');
}

Device::Device()
{
  busy=false;
  nb_command=0;
  client_id=-1;
  thread_pid=-1;
}

Device::~Device()
{

}

bool Device::start_loop()
{
  if(this->busy)
    return false;
  this->busy = true;
  std::thread(Device::loop, this).detach();
  return true;
}

std::pair<int32_t,int32_t> importRange(const ztd::chunkdat& ch, const std::string& tag, int32_t low, int32_t high)
{
  ztd::chunkdat* pch=ch.subChunkPtr(tag);
  if(pch != nullptr)
  {
    std::string str=*pch;
    auto tpos=str.find(':');
    if (str=="*") //whole range
    {
      return std::make_pair(low,high);
    }
    else if(tpos == std::string::npos) //single value
    {
      low=std::stoi(str);
      high=low;
    }
    else //range
    {
      low=std::stoi(str.substr(0,tpos));
      tpos++;
      high=std::stoi(str.substr(tpos, str.size()-tpos));
    }
  }
  return std::make_pair(low, high);
}

std::pair<float,float> importRangeFloat(const ztd::chunkdat& ch, const std::string& tag, float low, float high)
{
  ztd::chunkdat* pch=ch.subChunkPtr(tag);
  if(pch != nullptr)
  {
    std::string str=*pch;
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

bool importBool(const ztd::chunkdat& ch, const std::string& tag, const bool& defbool)
{
  ztd::chunkdat* pch=ch.subChunkPtr(tag);
  if(pch != nullptr)
  {
    std::string str=*pch;
    if( str == "true" )
      return true;
    else if( str == "false" )
      return false;
  }
  return defbool;
}

bool Device::import_chunk(const ztd::chunkdat& ch)
{
  ztd::chunkdat& cch = ch["commands"];
  this->name=dequote(ch["name"]);
  for(int i=0 ; i<cch.listSize() ; i++)
  {
    ztd::chunkdat& tch=cch[i];
    std::string tstr=tch["type"];
    if(tstr == "system") //type system
    {
      std::string shell;
      shell=tch["shell"];
      this->sysCommands.push_back(SystemCommand(shell));
    }
    else if (tstr == "connect")
    {
      std::string shell;
      shell=tch["shell"];
      this->connectCommands.push_back(ConnectCommand(shell));
    }
    else if (tstr == "disconnect")
    {
      std::string shell;
      shell=tch["shell"];
      this->disconnectCommands.push_back(DisconnectCommand(shell));
    }
    else
    {
      int8_t channel;
      std::string shell;

      std::pair<int32_t,int32_t> intpair;
      std::pair<float,float> floatpair;

      //channel
      if(tch.subChunkPtr("channel") == nullptr || tch["channel"] == "*")
        channel=-1;
      else
        channel=std::stoi(tch["channel"]);

      //shell
      shell=tch["shell"];

      //type
      if(tstr == "note") //type note
      {
        uint8_t id_low=0;
        uint8_t id_high=127;
        uint8_t low=1;
        uint8_t high=127;

        //id
        intpair=importRange(tch, "id", id_low, id_high);
        id_low=intpair.first;
        id_high=intpair.second;

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
    kill(this->thread_pid, SIGINT);
    this->busy=false;
    this->client_id=-1;
  }

  else if (index(buff, ':') != NULL) // MIDI command
  {
    if (strstr(buff, "System exclusive") != NULL) //system exclusive
    {
      char* val=buff+35;
      std::string strval;
      while(*val != '\n')
      {
        if(*val != ' ')
          strval += *val;
        val++;
      }

      for( auto it : this->sysCommands )
      {
        std::string command="code=" + strval + ";";
        command += dequote(it.shell);
        std::thread(ztd::sh, command, true).detach();
      }
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
        // throw std::runtime_error("Unknown MIDI signal:\n" + std::string(buff));
        std::cerr << "Unsupported signal, ignoring\n" ;
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
        while ( _isNum(*(pos+t)) )
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
        while ( _isNum(*(pos+t)) )
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
            command += dequote(it.shell);
            std::thread(ztd::sh, command, true).detach();
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
            command += ";" + dequote(it.shell);
            std::thread(ztd::sh, command, true).detach();
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
            std::string command="channel=" + std::to_string(channel)
            + ";rawvalue=" + std::to_string(value)
            + ";value=";
            if(it.floating)
              command += std::to_string(result);
            else
              command += std::to_string((long int) result);
            command += ";" + dequote(it.shell);
            std::thread(ztd::sh, command, true).detach();
          }
        }
      } // if type

    } //if system exclusive

  } //while

}

void Device::loop(Device* dev)
{
  char* buff = NULL;
  size_t buff_size = 0;
  std::string command = "aseqdump -p '" + std::to_string(dev->client_id) + '\'';
  FILE *stream = ztd::popen2(command.c_str(), "r", &dev->thread_pid);

  log("Device '" + dev->name + "' connected\n");

  for( auto it : dev->connectCommands )
  {
    std::thread(ztd::sh, dequote(it.shell), true).detach();
  }

  while (getline(&buff, &buff_size, stream) > 0)
  {
    dev->run_signal(buff);
  }

  for( auto it : dev->disconnectCommands )
  {
    std::thread(ztd::sh, dequote(it.shell), true).detach();
  }

  log("Device '" + dev->name + "' disconnected\n");

  ztd::pclose2(stream, dev->thread_pid);
  dev->thread_pid=-1;
  free(buff);
}

void clean_devices()
{
  for(auto it : device_list)
  {
    delete it;
  }
  device_list.clear();
}

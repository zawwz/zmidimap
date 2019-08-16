#include "format.hpp"

static void _skipline(const std::string& mim, unsigned int& i)
{
  while(i<mim.size() && mim[i] != '\n') {
    i++;
  }
}

static void _skipread(const std::string& mim, unsigned int& i)
{
  while(i<mim.size())
  {
    if(i+1<mim.size() && mim[i] == '/' && mim[i+1] == '/') {
      _skipline(mim, i);
    }
    if(ztd::filedat::isRead(mim[i])) {
      break;
    }
    i++;
  }
}

static void _find_next(const std::string& mim, unsigned int& i, const std::string& str)
{
  while(i<mim.size())
  {
    _skipread(mim, i);
    if(i+str.size() < mim.size() && mim.substr(i, str.size()) == str)
    {
      break;
    }
    else
    {
      _skipline(mim, i);
    }
  }
}

std::string file_strimport(const std::string& path)
{
  std::ifstream file(path);
  std::string ret, line;
  while(file)
  {
    getline(file, line);
    ret += line + '\n';
  }
  return ret;
}

ztd::chunkdat mimtochk_commands(const std::string& mim)
{
  ztd::chunkdat chk;
  unsigned int i=0,j=0;
  _find_next(mim,i, "Command ");
  while(i<mim.size())
  {
    i+=7;
    j=i;
    _skipline(mim,j);
    std::string type=mim.substr(i,j-i);
    j++;
    i=j;
    _find_next(mim,j, "Command ");
    std::string flags=mim.substr(i,j-i);
    i=j;

    chk.add("{type=" + type + '\n' + flags + '}');
  }
  return chk;
}

ztd::chunkdat mimtochk(const std::string& mim)
{
  ztd::chunkdat chk;
  unsigned int i=0,j=0;
  _find_next(mim,i, "Device ");
  while(i<mim.size())
  {
    ztd::chunkdat device;
    i+=7;
    j=i;
    _skipline(mim,j);
    std::string name=mim.substr(i,j-i);
    j++;
    i=j;
    _find_next(mim,j, "Device ");
    std::string commands=mim.substr(i,j-i);
    i=j;

    device.add("name", name);
    device.add("commands", mimtochk_commands(commands));

    chk.add(device);
  }
  return chk;
}

bool is_mim(const std::string& str)
{
  unsigned int i=0;
  _skipread(str, i);
  return str.substr(i,7) == "Device ";
}

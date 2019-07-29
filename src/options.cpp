#include "options.hpp"

#include <iostream>

std::vector<std::string> argVector(int argc, char** argv)
{
  std::vector<std::string> out;
  for(int i=1;i<argc; i++)
  {
    out.push_back(std::string(argv[i]));
  }
  return out;
}

Option::Option()
{
  shortDef=false;
  longDef=false;
  takesArgument=false;
  activated=false;
  charName=0;
}
Option::~Option()
{
}

Option::Option(char c, bool arg, std::string helptext, std::string argname)
{
  shortDef=true;
  longDef=false;
  takesArgument=arg;
  activated=false;
  charName=c;
  arg_name=argname;
  help_text=helptext;
}

Option::Option(std::string const& str, bool arg, std::string helptext, std::string argname)
{
  shortDef=false;
  longDef=true;
  takesArgument=arg;
  activated=false;
  charName=0;
  strName=str;
  arg_name=argname;
  help_text=helptext;
}
Option::Option(char c, std::string const& str, bool arg, std::string helptext, std::string argname)
{
  shortDef=true;
  longDef=true;
  takesArgument=arg;
  activated=false;
  charName=c;
  strName=str;
  arg_name=argname;
  help_text=helptext;
}

void Option::printHelp(int leftpad, int rightpad)
{
  //prepadding
  printf("%*s", -1*leftpad, "");

  //short def
  if(this->shortDef)
  {
    printf("-%c ", this->charName);
    rightpad -= 3;
  }

  //longdef
  if(this->longDef)
  {
    printf("--%s ", this->strName.c_str());
    rightpad -= 3 + this->strName.size();
  }

  //argument
  if(this->takesArgument)
  {
    printf(" <%s>", arg_name.c_str());
    rightpad -= arg_name.size() + 3;
  }

  printf("%*s%s", -1*rightpad, "", help_text.c_str());

  printf("\n");
}

void OptionSet::printHelp(int leftpad, int rightpad)
{
  for(auto it : this->m_options)
  {
    it.printHelp(leftpad,rightpad);
  }
}

OptionSet::OptionSet()
{
  errStream = &(std::cerr);
}

OptionSet::~OptionSet()
{
}

Option* OptionSet::findOption(char c)
{
  for( auto it=m_options.begin() ; it!=m_options.end() ; it++ )
  {
    if((*it).shortDef && (*it).charName == c)
      return &(*it);
  }
  return nullptr;
}
Option* OptionSet::findOption(std::string const& str)
{
  for( auto it=m_options.begin() ; it!=m_options.end() ; it++ )
  {
    if((*it).longDef && (*it).strName == str)
    return &(*it);
  }
  return nullptr;
}

std::pair<std::vector<std::string>, bool> OptionSet::getOptions(std::vector<std::string> input)
{
  std::vector<std::string> out;
  unsigned int i=0;
  for( auto it = input.begin(); it!=input.end() ; it++ )
  {
    if( (*it).size()>0 && (*it)[0]=='-' )
    {
      if((*it).size()>1 && (*it)[1]=='-')
      {
        std::size_t eqn=(*it).find('=');
        if(eqn == std::string::npos)
        {
          Option* popt = this->findOption( (*it).substr( 2,(*it).size()-2) );
          if(popt == nullptr)
          {
            (*errStream) << "Unknown option: " << (*it).substr(0,eqn) << std::endl;
            return std::make_pair(out, false);
          }
          if(popt->takesArgument)
          {
            if( ++it == input.end() ) //finishes here
            {
              (*errStream) << "No argument given to option --" << popt->strName << std::endl;
              return std::make_pair(out, false);
            }
            popt->activated = true;
            popt->argument = (*it);
          }
          else
          {
            popt->activated = true;
          }
        }
        else
        {
          Option* popt = this->findOption( (*it).substr(2,eqn-2) );
          if(popt == nullptr)
          {
            (*errStream) << "Unknown option: " << (*it).substr(2,eqn-2) << std::endl;
            return std::make_pair(out, false);
          }
          if(!popt->takesArgument)
          {
            (*errStream) << "Option " << popt->strName << " doesn't take an argument" << std::endl;
            return std::make_pair(out, false);
          }
          popt->argument = (*it).substr(eqn+1,(*it).size()-eqn-1 );
        }
      }
      else
      {
        i=1;
        Option* popt=nullptr;
        bool tstop=false;
        while( !tstop && it!=input.end() && (*it).size()>i )
        {
          popt=this->findOption((*it)[i]);
          if(popt==nullptr) //not found: error
          {
            (*errStream) << "Unknown option: -" << (*it)[i] << std::endl;
            return std::make_pair(out, false);
          }
          if(popt->takesArgument) //no argument
          {
            i++;
            if((*it).size()<=i) //finishes here
            {
              if( ++it == input.end() )
              {
                (*errStream) << "No argument given to option -" << popt->charName << std::endl;
                return std::make_pair(out, false);
              }
              popt->activated = true;
              popt->argument = (*it);
              tstop = true;
            }
            else //continue
            {
              if( (*it)[i] != '=') //incorrect
              {
                (*errStream) << "No argument given to option -" << popt->charName << std::endl;
                return std::make_pair(out, false);
              }
              i++;
              popt->argument = (*it).substr(i , (*it).size()-i );
              popt->activated = true;
              tstop=true;
            }
          }
          else //no argument
          {
            popt->activated = true;
          }

          i++;
        }
      }
    }
    else
    {
      out.push_back(*it);
    }
    if(it == input.end())
      break;
  }
  return std::make_pair(out, true);
}

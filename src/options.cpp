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

Option::Option(char c, bool arg)
{
  shortDef=true;
  longDef=false;
  takesArgument=arg;
  activated=false;
  charName=c;
}

Option::Option(std::string const& str, bool arg)
{
  shortDef=false;
  longDef=true;
  takesArgument=arg;
  activated=false;
  charName=0;
  strName=str;
}
Option::Option(char c, std::string const& str, bool arg)
{
  shortDef=true;
  longDef=true;
  takesArgument=arg;
  activated=false;
  charName=c;
  strName=str;
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
            if( ++it == input.end() ) //se termine ici
            {
              (*errStream) << "No argument given to option " << popt->strName << std::endl;
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
          if(popt==nullptr) //non trouvé: erreur
          {
            (*errStream) << "Unknown option: " << (*it)[i] << std::endl;
            return std::make_pair(out, false);
          }
          if(popt->takesArgument) //prends un argument
          {
            i++;
            if((*it).size()<=i) //se termine ici
            {
              if( ++it == input.end() ) //se termine ici
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
          else // ne prends pas d'argument
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

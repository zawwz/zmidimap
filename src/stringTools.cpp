#include "stringTools.hpp"

uint8_t hexStringToByte(std::string const& in)
{
  if(in.size() == 2)
  {
    return hexDigitToNum(in[0])*16 + hexDigitToNum(in[1]);
  }
  if(in.size() == 1)
  {
    return hexDigitToNum(in[0]);
  }
  else
    return 0;
}

int8_t hexDigitToNum(char car)
{
  if(isNum(car))
    return car - '0';
  else if(car>='a' && car<='f')
    return car - 'a' + 10;
  else if(car>='A' && car<='F')
    return car - 'A' + 10;
  else
    return -1;
}

std::string byteToHexString(uint8_t const& byte)
{
  std::string ret;
  ret += toHexDigit(byte/16);
  ret += toHexDigit(byte%16);
  return ret;
}

char toHexDigit(uint8_t const& num)
{
  if(num>=0 && num<=9)
  {
    return '0' + num;
  }
  else if(num < 16)
  {
    return 'A' + num - 10;
  }
  else
    return 0;
}

std::string repeatString(std::string const& a, unsigned int n)
{
  std::string ret;
  while(n>0)
  {
    ret += a;
    n--;
  }
  return ret;
}

std::string repeatChar(char a, unsigned int n)
{
  std::string ret;
  while(n>0)
  {
    ret.push_back(a);
    n--;
  }
  return ret;
}

bool isAlpha(char a)
{
  return (a>='a' && a<='z') || (a>='A' && a<='Z');
}

bool isNum(char a)
{
  return (a>='0' && a<='9');
}

bool isHexdec(char a)
{
  return isNum(a) || (a>='a' && a<='f') || (a>='A' && a<='F');
}

bool equalNoCase(char a, char b)
{
  if(a==b)
    return true;
  if(!isAlpha(a) || !isAlpha(b))
    return false;
  return (a-b == 'a' - 'A') || (a-b == 'A' - 'a');
}

bool equalNoCase(std::string const& st1, std::string const& st2)
{
  if(st1.size()!=st2.size())
    return false;
  for(unsigned int i=0; i<st1.size(); i++)
  {
    if(!equalNoCase(st1[i],st2[i]))
      return false;
  }
  return true;
}

bool hasCommonChar(std::string const& str1, std::string const& str2)
{
  if(str1.size() < str2.size())
  {
    for(std::string::const_iterator it = str1.begin(); it!= str1.end(); it++)
      if(str2.find(*it))
        return true;
  }
  else
    for(std::string::const_iterator it = str2.begin(); it!= str2.end(); it++)
      if(str1.find(*it))
        return true;
  return false;
}

std::string fuse(std::vector<std::string> const& vec, int first, int last, char separator)
{
  if(last<0 || last >= (int) vec.size())
    last=vec.size()-1;
  if(first<0)
    first=0;
  std::string ret;
  if(first <= last)
  {
    for(int i=first; i<=last-1; i++)
    {
      ret+=vec[i];
      if(separator != 0)
      ret+=separator;
    }
    ret += vec[last];
  }
  return ret;
}

int findInString(std::string const& in, char const find)
{
  for(unsigned int i=0; i<in.size(); i++)
  {
    if(in[i]==find)
    return i;
  }
  return -1;
}

unsigned int charCount(std::string const& in, char const find)
{
  return std::count(in.begin(), in.end(), find);
}

bool isRead(char in)
{
    bool out=false;
    if(in>=33 && in<=126)
        out=true;
    return out;
}

std::string decapsulate(std::string const& in)
{
  unsigned int i=0,j;
  while(i<in.size() && !isRead(in[i]))
    i++;
  j=i;
  if(i<in.size() && in[i]=='\"')
  {
    i++;
    j++;
    while(i<in.size() && in[i]!='\"')
    {
      if(in[i]=='\\')
        i++;
      i++;
    }
  }
  else if(i<in.size() && in[i]=='[')
  {
      unsigned int counter=0;
      i++;
      j++;
      while(i<in.size() && !(in[i]==']' && counter==0))
      {
          if(in[i]=='\\')
            i++;
          if(in[i]=='[')
            counter++;
          if(in[i]==']')
            counter--;
          i++;
      }
  }
  else if(i<in.size() && in[i]=='{')
  {
      unsigned int counter=0;
      i++;
      j++;
      while(i<in.size() && !(in[i]=='}' && counter==0))
      {
          if(in[i]=='\\')
            i++;
          if(in[i]=='{')
            counter++;
          if(in[i]=='}')
            counter--;
          i++;
      }
  }
  else if(i<in.size() && in[i]=='(')
  {
      unsigned int counter=0;
      i++;
      j++;
      while(i<in.size() && !(in[i]==')' && counter==0))
      {
          if(in[i]=='\\')
            i++;
          if(in[i]=='(')
            counter++;
          if(in[i]==')')
            counter--;
          i++;
      }
  }
  else if(i<in.size() && in[i]=='<')
  {
      unsigned int counter=0;
      i++;
      j++;
      while(i<in.size() && !(in[i]=='>' && counter==0))
      {
          if(in[i]=='\\')
            i++;
          if(in[i]=='<')
            counter++;
          if(in[i]=='>')
            counter--;
          i++;
      }
  }
  else
  {
    i=in.size()-1;
    while(i<in.size() && i>0 && !isRead(in[i]))
    {
      i--;
    }
    i++;
  }
  return in.substr(j,i-j);
}

std::string ridUnread(std::string const& in)
{
  int i=0,j=in.size()-1;
  while( ((unsigned int) i)<in.size() && !isRead(in[i]))
    i++;
  while(j>=0 && !isRead(in[j]))
    j--;
  if(i<=j)
    return in.substr(i, j-i+1);
  else
    return "";
}

std::string readInString(std::string const& in, std::string const& name, unsigned int occurence)
{
    bool found=false;
    std::string out="";
    unsigned int i=0,j=0,reading=0;
    while(i<in.size() && !found)
    {
        if(!isRead(in[i])) /*Lire jusqu'à un caractère lu (ingorer les espaces etc...)*/
            i++;
        else if(i<in.size()+name.size()+1 && (in.substr(i,name.size()+1)==name+"=" || (in.substr(i,name.size()+1)==name+":") ) ) /*La valeur qu'on veut*/
        {
            if(occurence==reading) /*Si c'est l'occurence que l'on veut*/
            {
                unsigned int counter=0;
                i+=name.size()+1;
                j=i;
                if(in[i]=='{') /*Valeur encapsulée*/
                {
                    i++;
                    j++;
                    while(i<in.size() && !(in[i]=='}' && counter==0))
                    {
                        if(in[i]=='\\')
                            i++;
                        else if(in[i]=='{')
                            counter++;
                        else if(in[i]=='}')
                            counter--;
                        i++;
                    }
                }
                else if(in[i]=='\"')
                {
                    i++;
                    j++;
                    while(i<in.size() && in[i]!='\"')
                    {
                        if(in[i]=='\\')
                            i++;
                        i++;
                    }
                }
                else /*Valeur non encapsulée: jusqu'à \n*/
                {
                    i++;
                    while(i<in.size() && in[i]!='\n')
                        i++;
                }
                found=true;
                out=in.substr(j,i-j);
            }
            else /*On veut une autre occurence: passer cette valeur là*/
            {
              reading++;
              i+=name.size()+1;
              unsigned int counter=0;
              if(i<in.size() && in[i]=='{') /*Encapsulée*/
              {
                i++;
                while(i<in.size() && !(in[i]=='}' && counter==0))
                {
                    if(in[i]=='\\')
                      i++;
                    if(in[i]=='{')
                      counter++;
                    if(in[i]=='}')
                      counter--;
                    i++;
                }
              }
              else if(in[i]=='\"')
              {
                  i++;
                  while(i<in.size() && in[i]!='\"')
                  {
                      if(in[i]=='\\')
                          i++;
                      i++;
                  }
              }
              else /*Jusqu'à \n*/
                while(i<in.size() && in[i]!='\n')
                  i++;

            }
        }
        else /*Pas la valeur qu'on veut*/
        {
          while(i<in.size() && in[i]!='=' && in[i]!='\n' && in[i]!=' ' && in[i]!='\t' && in[i]!='\r')
            i++;
          if(i<in.size() && in[i]=='=')
            i++;
          if(i<in.size() && in[i]=='{')
          {
            unsigned int counter=0;
            i++;
            while(i<in.size() && !(in[i]=='}' && counter==0))
            {
                if(in[i]=='\\')
                  i++;
                if(in[i]=='{')
                  counter++;
                if(in[i]=='}')
                  counter--;
                i++;
            }
          }
          else if(i<in.size() && in[i]=='\"')
          {
            i++;
            while(i<in.size() && in[i]!='\"')
            {
                if(in[i]=='\\')
                    i++;
                i++;
            }
          }
          i++;
        }
    }
    return out;
}

std::pair<std::string,std::string> readValue(std::string const& in, unsigned int occurence, int* rank)
{
  std::string name="";
  unsigned int i=0,j=0,reading=0;
  while(i<in.size())
  {
    while(!isRead(in[i])) /*Lire jusqu'à un caractère lu (ingorer les espaces etc...)*/
      i++;
    if(i>=in.size())
    {
      if(rank != nullptr)
        *rank=-1;
      return std::make_pair("","");
    }
    while(in[i]!='=' && in[i]!=':') //lire le nom
    {
      name += in[i];
      i++;
    }
    i++;
    if(occurence==reading) /*Si c'est l'occurence que l'on veut*/
    {
      unsigned int counter=0;
      j=i;
      if(in[i]=='{') /*Valeur encapsulée*/
      {
        i++;
        while(i<in.size() && !(in[i]=='}' && counter==0))
        {
          if(in[i]=='\\')
            i++;
          else if(in[i]=='\"') //skip quotes
          {
            i++;
            while(i<in.size() && in[i]!='\"') i++;
            i++;
          }
          else if(in[i]=='{')
            counter++;
          else if(in[i]=='}')
            counter--;
          i++;
        }
        i++;
        if(rank!=nullptr)
          *rank=i;
        return std::make_pair(name, in.substr(j,i-j));
      }
      else if(in[i]=='[') /*Valeur encapsulée*/
      {
        i++;
        while(i<in.size() && !(in[i]==']' && counter==0))
        {
          if(in[i]=='\\')
            i++;
          else if(in[i]=='\"') //skip quotes
          {
            i++;
            while(i<in.size() && in[i]!='\"') i++;
            i++;
          }
          else if(in[i]=='[')
            counter++;
          else if(in[i]==']')
            counter--;
          i++;
        }
        i++;
        if(rank!=nullptr)
          *rank=i;
        return std::make_pair(name, in.substr(j,i-j));
      }
      else if(in[i]=='\"')
      {
        i++;
        j++;
        while(i<in.size() && in[i]!='\"')
        {
          if(in[i]=='\\')
            i++;
          i++;
        }
        if(rank!=nullptr)
          *rank=i+1;
        std::string ret;
        ret += '\"';
        ret += in.substr(j,i-j);
        ret += '\"';
        return std::make_pair(name, ret);
      }
      else /*Valeur non encapsulée: jusqu'à \n*/
      {
        i++;
        while(i<in.size() && in[i]!='\n')
          i++;
        if(rank!=nullptr)
          *rank=i;
        if(i>in.size())
          i=in.size();
        if(j>in.size())
          j=in.size();
        return std::make_pair(name, in.substr(j,i-j));
      }
    }
    else /*On veut une autre occurence: passer cette valeur là*/
    {
      reading++;
      i+=name.size()+1;
      unsigned int counter=0;
      if(i<in.size() && in[i]=='{') /*Encapsulée*/
      {
        i++;
        while(i<in.size() && !(in[i]=='}' && counter==0))
        {
          if(in[i]=='\\')
            i++;
          if(in[i]=='\"') //quote skip
          {
            i++;
            while(i<in.size() && in[i]!='\"') i++;
          }
          if(in[i]=='{')
            counter++;
          if(in[i]=='}')
            counter--;
          i++;
        }
      }
      else if(in[i]=='\"') //quote
      {
        i++;
        while(i<in.size() && in[i]!='\"')
        {
          if(in[i]=='\\')
          i++;
          i++;
        }
      }
      else /*Jusqu'à \n*/
      while(i<in.size() && in[i]!='\n')
        i++;
    }
  }
  if(rank!=nullptr)
    *rank=-1;
  return std::make_pair("","");
}

std::vector<std::string> readList(std::string const& in)
{
    unsigned int i=0,j=0,counter=0;
    std::vector<std::string> out;
    while(i<in.size() && in[i]!='[')
        i++;
    if(in[i]=='[')
    {
        i++;
        while(i<in.size() && in[i]!=']' && counter==0)
        {
            j=i;
            while(i<in.size() && !(in[i]==',' && counter==0) && !(in[i]==']' && counter==0) )
            {
                if(in[i]=='\\')
                    i++;
                else if(in[i]=='{')
                {
                    i++;
                    unsigned int counter2=0;
                    while( i<in.size() && (in[i]!='}' || counter2!=0) )
                    {
                        if(in[i]=='\\')
                            i++;
                        else if(in[i]=='{')
                            counter2++;
                        else if(in[i]=='}')
                            counter2--;
                        i++;
                    }
                }
                else if(in[i]=='\"')
                {
                    i++;
                    while(in[i]!='\"')
                    {
                        if(in[i]=='\\')
                            i++;
                        i++;
                    }
                }
                else
                {
                    if(i<in.size() && in[i]=='[')
                        counter++;
                    if(i<in.size() && in[i]==']')
                        counter--;
                }
                i++;
            }
            if((in[i]==',' || in[i]==']') && counter==0)
                out.push_back(ridUnread(in.substr(j,i-j)));
            else
                out.push_back(ridUnread(in.substr(j,i-j+1)));
            if(in[i]==',')
                i++;
        }
    }
    if ( out.size() == 1 && out[0]=="" )
      return std::vector<std::string>();
    return out;
}

std::vector<std::string> splitString(std::string const& in, char const delim, char encapsulator, char decapsulator)
{
    std::vector<std::string> out;
    if(delim!='\\' && delim!=encapsulator && delim!=decapsulator)
    {
        if(decapsulator==0)
            decapsulator=encapsulator;
        unsigned int i=0;
        while(i<in.size())
        {
            std::string t="";
            while(i<in.size() && in[i]!=delim)
            {
                if(i<in.size() && in[i]==encapsulator)
                {
                    i++;
                    while(i<in.size() && in[i]!=decapsulator)
                    {
                        if(in[i]=='\\')
                        {
                            i++;
                            t.push_back('\\');
                            t.push_back(in[i]);
                            i++;
                        }
                        else
                        {
                            t.push_back(in[i]);
                            i++;
                        }
                    }
                    i++;
                }
                else if(in[i]=='\\')
                {
                    i++;
                    if(i<in.size() && in[i]=='n')
                    {
                        t.push_back(10);
                    }
                    else
                        t.push_back(in[i]);
                    i++;
                }
                else
                {
                    t.push_back(in[i]);
                    i++;
                }
            }
            out.push_back(t);
            i++;
        }
    }
    return out;
}

int replaceChar(std::string& in, char const f,char const r)
{
    int j=0;
    for(unsigned int i=0;i<in.size();i++)
        if(in[i]==f)
        {
            in[i]=r;
            j++;
        }
    return j;
}

bool isIn(char const c, std::string const& str)
{
  return str.find(c) != std::string::npos;
}

std::string encompact(std::string const& in, std::string const& chars, char const into)
{
    std::string out;
    unsigned int i=0,j=0;
    while(i<in.size())
    {
        j=i;
        while(isIn(in[i],chars))
            i++;
        if(j!=i)
        {
            out.push_back(into);
        }
        else
        {
            out.push_back(in[i]);
            i++;
        }
    }
    return out;
}

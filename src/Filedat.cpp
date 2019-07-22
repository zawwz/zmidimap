#include "Filedat.hpp"

#include <exception>

bool _isRead(char in)
{
    bool out=false;
    if(in>=33 && in<=126)
        out=true;
    return out;
}

std::string _repeatString(const std::string& str, const unsigned int n)
{
  std::string ret;
  for(unsigned int i=0 ; i<n ; i++)
    ret += str;
  return ret;
}

void printFileException(file_format_error& exc)
{
  printErrorIndex(exc.data(), exc.where(), exc.what(), exc.origin());
}

void printErrorIndex(const char* in, const int index, const std::string& message, const std::string& origin)
{
  int i=0, j=0; // j: last newline
  int line=1; //n: line #
  int in_size=strlen(in);
  while(i < in_size && i < index)
  {
    if(in[i] == '\n')
    {
      line++;
      j=i+1;
    }
    i++;
  }
  while(i < in_size && in[i]!='\n')
  {
    i++;
  }
  std::cerr << origin << ": Error\nLine " << line << " col " << index-j << ": " << message << std::endl;
  std::cerr << std::string(in+j, i-j) << std::endl;
  std::cerr << _repeatString(" ", index-j) << '^' << std::endl;
}


Filedat::Filedat()
{
  m_dataChunk = nullptr;
}

Filedat::Filedat(std::string const& in)
{
  m_dataChunk = nullptr;
  m_filePath=in;
}

Filedat::~Filedat()
{
  this->clear();
}

void Filedat::clear()
{
  if(m_dataChunk!=nullptr)
  {
    delete m_dataChunk;
    m_dataChunk = nullptr;
  }
}

bool Filedat::readTest() const
{
  std::ifstream stream(m_filePath);
  if(!stream)
    return false;
  else
    return true;
}

void Filedat::importFile()
{
  std::ifstream stream(m_filePath);
  if(!stream)
  {
    throw std::runtime_error("Cannot open file '" + m_filePath + '\'');
  }

  m_data="";
  std::string line;

  while(stream)
  {
    getline(stream, line);
    m_data += (line + '\n');
  }

  this->clear();
  try
  {
    m_dataChunk = new Chunk(m_data.c_str(), m_data.size(), 0, this);
  }
  catch(chunk_format_error& e)
  {
    throw file_format_error(e.what(), m_filePath, m_data.c_str(), e.where());
  }
}

bool Filedat::exportFile(std::string const& path, std::string const& aligner) const
{
  std::ofstream stream;
  if(path=="")
    stream.open(m_filePath);
  else
    stream.open(path);
  if(!stream)
    return false;
  stream << this->strval(aligner);
  return true;
}

std::string Filedat::strval(std::string const& aligner) const
{
  if(m_dataChunk == nullptr)
    return "";
  else
    return m_dataChunk->strval(0, aligner);
}

std::string _getname(const char* in, const int in_size, int* start, int* val_size, int* end)
{
  int i=0;

  *start = in_size; //default no value
  *end = in_size; //default end
  *val_size=0; //default no value

  while(i<in_size)
  {
    if(i+1<in_size && in[i] == '/' && in[i+1] == '/')
      while(i<in_size && in[i] != '\n')
        i++;

    if(_isRead(in[i]))
      break;

    i++;
  }
  if(i >= in_size) //ends without value
    return "";

  int j=i; //name start
  while(i<in_size && in[i] != '=') //skip to =
    i++;
  if(i >= in_size) //no =
  {
    throw chunk_format_error("Tag has no value", j);
  }

  if(i == j) //nothing preceding =
    throw chunk_format_error("Value has no tag", i);

  int k=i-1; //name end
  while( !_isRead(in[k]) )
    k--;
  std::string name=std::string(in+j, k-j+1);

  i++;
  while(i < in_size && !_isRead(in[i]))
    i++;
  if(i >= in_size) //no value
  {
    *start=i;
    *val_size=0;
    *end=i;
    return name;
  }
  if(in[i] == '\"') //"" val
  {
    i++;
    *start=i; //value starts
    j=0; //size
    while(i+j < in_size && in[i+j]!='\"')
    {
      if(in[i]+j=='\\')
        j++;
      j++;
    }
    if(i+j >= in_size) // no closing "
      throw chunk_format_error("Double quote does not close", i-1);
    *val_size=j;
    *end=i+j+1;
    return name;
  }
  if(in[i] == '\'') //"" val
  {
    i++;
    *start=i; //value starts
    j=0; //size
    while(i+j < in_size && in[i+j]!='\'')
    {
      if(in[i]+j=='\\')
        j++;
      j++;
    }
    if(i+j >= in_size) // no closing '
      throw chunk_format_error("Single quote does not close", i-1);
    *val_size=j;
    *end=i+j+1;
    return name;
  }
  if(in[i] == '{')
  {
    *start=i;
    j=1;
    int counter=0;
    while( i+j < in_size && !( counter == 0 && in[i+j]=='}') )
    {
      if(i+j+1<in_size && in[i+j] == '/' && in[i+j+1] == '/')
        while(i+j<in_size && in[i+j] != '\n')
          j++;
      if(in[i+j]=='\\')
        j++;
      if(in[i+j]=='{')
        counter++;
      if(in[i+j]=='}')
        counter--;
      j++;
    }
    if(i+j >= in_size) //reached end without closing
      throw chunk_format_error("Brace does not close", i);
    j++;
    *val_size=j;
    *end=i+j;
    return name;
  }
  if(in[i] == '[')
  {
    *start=i;
    j=1;
    int counter=0;
    while( i+j < in_size && !( counter == 0 && in[i+j]==']') )
    {
      if(i+j+1<in_size && in[i+j] == '/' && in[i+j+1] == '/')
        while(i+j<in_size && in[i+j] != '\n')
          j++;
      if(in[i+j]=='\\')
        j++;
      if(in[i+j]=='[')
        counter++;
      if(in[i+j]==']')
        counter--;
      j++;
    }
    if(i+j >= in_size) //reached end without closing
      throw chunk_format_error("Bracket does not close", i);
    j++;
    *val_size=j;
    *end=i+j;
    return name;
  }
  { // no encapsulation: go to end of line
    *start=i; //value starts
    j=0; //size
    while(i+j < in_size && in[i+j]!='\n')
    {
      if(in[i]+j=='\\')
        j++;
      j++;
    }
    while( !_isRead(in[i+j]) )
      j--;
    *val_size=j+1;
    *end=i+j+1;
    return name;
  }

  return name;
}

std::string _getlist(const char* in, const int in_size, int* start, int* end)
{
  int i=0;
  std::string ret;

  while(i<in_size)
  {
    if(i+1<in_size && in[i] == '/' && in[i+1] == '/')
      while(i<in_size && in[i] != '\n')
        i++;

    if(_isRead(in[i]))
      break;

    i++;
  }

  *start=i;
  if(i >= in_size) //ends without value
  {
    *end = in_size;
    return "";
  }
  if(in[i] == ',') //value is empty
  {
    *end=i+1;
    return "";
  }

  int j=0;
  if(in[i] == '\"') //"" val
  {
    i++;
    j=0; //size
    while(i+j < in_size && in[i+j]!='\"')
    {
      if(in[i]+j=='\\')
        j++;
      j++;
    }
    if(i+j >= in_size) // no closing "
      throw chunk_format_error("Double quote does not close", i-1);
    ret = std::string(in+i, j);
    *end=i+j+1;
  }
  else if(in[i] == '\'') //"" val
  {
    i++;
    j=0; //size
    while(i+j < in_size && in[i+j]!='\'')
    {
      if(in[i]+j=='\\')
        j++;
      j++;
    }
    if(i+j >= in_size) // no closing '
      throw chunk_format_error("Single quote does not close", i-1);
    ret = std::string(in+i, j);
    *end=i+j+1;
  }
  else if(in[i] == '{')
  {
    j=1;
    int counter=0;
    while( i+j < in_size && !( counter == 0 && in[i+j]=='}') )
    {
      if(i+j+1<in_size && in[i+j] == '/' && in[i+j+1] == '/')
        while(i+j<in_size && in[i+j] != '\n')
          j++;
      if(in[i+j]=='\\')
        j++;
      if(in[i+j]=='{')
        counter++;
      if(in[i+j]=='}')
        counter--;
      j++;
    }
    if(i+j >= in_size) //reached end without closing
      throw chunk_format_error("Brace does not close", i);
    j++;
    ret = std::string(in+i, j);
    *end=i+j;
  }
  else if(in[i] == '[')
  {
    j=1;
    int counter=0;
    while( i+j < in_size && !( counter == 0 && in[i+j]==']') )
    {
      if(i+j+1<in_size && in[i+j] == '/' && in[i+j+1] == '/')
        while(i+j<in_size && in[i+j] != '\n')
          j++;
      if(in[i+j]=='\\')
        j++;
      if(in[i+j]=='[')
        counter++;
      if(in[i+j]==']')
        counter--;
      j++;
    }
    if(i+j >= in_size) //reached end without closing
      throw chunk_format_error("Bracket does not close", i);
    j++;
    ret = std::string(in+i, j);
    *end=i+j;
  }
  else // no encapsulation: go to next ,
  {
    j=0; //size
    while(i+j < in_size && in[i+j]!=',')
    {
      if(in[i+j]=='\\')
        j++;
      j++;
    }
    if(i+j < in_size)
    {
      while( !_isRead(in[i+j]) )
      j--;
    }
    ret = std::string(in+i,j);
    *end=i+j;
  }

  i = *end;
  while(i < in_size && !_isRead(in[i]))
    i++;
  if( i>= in_size ) //last char
  {
    *end=i;
    return ret;
  }
  else if(in[i] ==',') //comma as expected
  {
    *end=i+1;
    return ret;
  }
  else //Unexpected char
    throw chunk_format_error("Expecting comma", i);

}

void Chunk::set(const char* in, const int in_size, int offset, Filedat* data)
{
  this->clear(); //reset everything
  this->m_parent=data;
  this->m_offset=offset;

  int i=0;

  while(i<in_size && !_isRead(in[i])) //skip unread char
    i++;

  if(i >= in_size) //empty: make an empty strval
  {
    DataVal* cv = new DataVal();
    m_achunk=cv;
    cv->val = "";
    return;
  }
  else if( in[i] == '{')
  {
    i++;
    int val_end=in_size-1;
    while(!_isRead(in[val_end])) //skip unread char
      val_end--;
    if(in[val_end] != '}')
      throw chunk_format_error("Expecting closing brace", val_end-1);

    DataChunk* tch = new DataChunk();
    m_achunk = tch;

    std::string name;
    std::string val;
    while(i < val_end)
    {
      int start=0;
      int _size=0;
      int end=0;

      std::string newstr=std::string(in+i, val_end-i);
      try
      {
        name = _getname(newstr.c_str(), newstr.size(), &start, &_size, &end);
        val = newstr.substr(start, _size);
      }
      catch(chunk_format_error& e)
      {
        throw chunk_format_error(e.what(), e.where()+i);
      }

      if( name == "" ) //no more values
        break;

      try
      {
        tch->values.insert(std::make_pair(name, new Chunk(val.c_str(),val.size(), offset + start+i, m_parent) ));
      }
      catch(chunk_format_error& e)
      {
        throw chunk_format_error(e.what(), e.where() + start + i );
      }

      i += end;
    }

    return;

  }
  else if( in[i] == '[')
  {
    i++;
    int val_end=in_size-1;
    while(!_isRead(in[val_end])) //skip unread char
      val_end--;
    if(in[val_end] != ']')
      throw chunk_format_error("Expecting closing bracket", val_end-1);

    ChunkList* tch = new ChunkList();
    m_achunk = tch;

    int end=0,start=0;
    while( i < val_end )
    {
      std::string val;
      std::string newstr=std::string(in+i, val_end-i);
      try
      {
        val = _getlist(newstr.c_str(), newstr.size(), &start, &end);
      }
      catch(chunk_format_error& e)
      {
        throw chunk_format_error(e.what(), e.where()+i);
      }

      try
      {
        tch->list.push_back(new Chunk(val.c_str(),val.size(), offset + start+i, m_parent) );
      }
      catch(chunk_format_error& e)
      {
        throw chunk_format_error(e.what(), e.where() + start + i );
      }

      i+=end;
    }

    return;

  }
  else // string value
  {
    int val_end=in_size;
    val_end--;
    while(!_isRead(in[val_end])) //skip unread char
      val_end--;

    DataVal* tch = new DataVal();
    m_achunk = tch;

    tch->val = std::string(in+i,val_end-i+1);

    return;

  }
}

bool Chunk::addToChunk(std::string const& name, Chunk const& val)
{
  if(this->type()==AbstractChunk::chunk)
  {
    DataChunk* cp = dynamic_cast<DataChunk*>(m_achunk);
    cp->values.insert(std::make_pair(name , new Chunk(val)));
    return true;
  }
  else if(this->type() == AbstractChunk::none)
  {
    DataChunk* cp = new DataChunk();
    cp->values.insert(std::make_pair(name , new Chunk(val)));
    m_achunk=cp;
    return true;
  }
  else
    return false;
}

bool Chunk::addToChunk(std::vector<std::pair<std::string, Chunk>> const& vec)
{
  if(this->type()!=AbstractChunk::chunk && this->type()!=AbstractChunk::none)
    return false;
  for(auto it : vec)
    this->add(it.first, it.second);
  return true;
}

bool Chunk::addToList(Chunk const& val)
{
  if(this->type()==AbstractChunk::list)
  {
    ChunkList* lp = dynamic_cast<ChunkList*>(m_achunk);
    lp->list.push_back(new Chunk(val));
    return true;
  }
  else if(this->type() == AbstractChunk::none)
  {
    ChunkList* lp = new ChunkList();
    lp->list.push_back(new Chunk(val));
    m_achunk=lp;
    return true;
  }
  else
    return false;
}

bool Chunk::addToList(std::vector<Chunk> const& vec)
{
  if(this->type()!=AbstractChunk::chunk && this->type()!=AbstractChunk::none)
    return false;
  for(auto it : vec)
    this->add(it);
  return true;
}

std::string Chunk::strval(unsigned int alignment, std::string const& aligner) const
{
  if(this->type()==AbstractChunk::val)
  {
    DataVal* vp = dynamic_cast<DataVal*>(m_achunk);
    return vp->val;
  }
  else if(this->type()==AbstractChunk::chunk)
  {
    DataChunk* cp = dynamic_cast<DataChunk*>(m_achunk);
    std::string ret="{\n";
    for(auto it : cp->values)
    {
      ret += _repeatString(aligner,alignment+1);
      ret += it.first;
      ret += '=';
      if(it.second!=nullptr)
        ret += it.second->strval(alignment+1, aligner);
      ret += '\n';
    }
    ret += _repeatString(aligner, alignment);
    ret += '}';
    return ret;
  }
  else if(this->type()==AbstractChunk::list)
  {
    ChunkList* lp = dynamic_cast<ChunkList*>(m_achunk);
    std::string ret="[\n";
    for(auto it : lp->list)
    {
      ret += _repeatString(aligner, alignment+1);
      if(it!=nullptr)
        ret += it->strval(alignment+1, aligner);
      ret += ",\n";
    }
    ret.erase(ret.end()-2);
    ret += _repeatString(aligner, alignment);
    ret += ']';
    return ret;
  }
  else
    return "";
}

int Chunk::listSize() const
{
  if(this->type() != AbstractChunk::list)
    return -1;
  ChunkList* cl = dynamic_cast<ChunkList*>(m_achunk);
  return cl->list.size();
}

Chunk* Chunk::subChunkPtr(std::string const& in) const
{
  if(this->type()==AbstractChunk::chunk)
  {
    DataChunk* dc = dynamic_cast<DataChunk*>(m_achunk);
    auto fi = dc->values.find(in);
    if(fi == dc->values.end()) //none found
      return nullptr;
    return fi->second;
  }
  else //not a chunk
  {
    return nullptr;
  }
}

Chunk* Chunk::subChunkPtr(unsigned int a) const
{
  if(this->type()==AbstractChunk::list)
  {
    ChunkList* cl = dynamic_cast<ChunkList*>(m_achunk);
    if(a >= cl->list.size()) //outside of range
      return nullptr;
    return cl->list[a];
  }
  else //not a list
  {
    return nullptr;
  }
}

Chunk& Chunk::subChunkRef(std::string const& in) const
{
  if(this->type()!=AbstractChunk::chunk)
  {
    if(m_parent != nullptr)
    {
      throw file_format_error("Element isn't a {}", m_parent->filePath(), m_parent->c_data(), m_offset );
    }
    else
    {
      throw chunk_format_error("Chunk isn't a {}", m_offset);
    }
  }
  DataChunk* dc = dynamic_cast<DataChunk*>(m_achunk);
  auto fi = dc->values.find(in);
  if(fi == dc->values.end())
  {
    if(m_parent != nullptr)
    {
      throw file_format_error("Chunk doesn't have '" + in + "' flag", m_parent->filePath(), m_parent->c_data(), m_offset );
    }
    else
    {
      throw chunk_format_error("Chunk doesn't have '" + in + "' flag", m_offset );
    }
  }
  return *fi->second;
}

Chunk& Chunk::subChunkRef(unsigned int a) const
{
  if(this->type()!=AbstractChunk::list)
  {
    if(m_parent != nullptr)
    {
      throw file_format_error("Element isn't a {}", m_parent->filePath(), m_parent->c_data(), m_offset );
    }
    else
    {
      throw chunk_format_error("Chunk isn't a {}", m_offset);
    }
  }
  ChunkList* cl = dynamic_cast<ChunkList*>(m_achunk);
  if(a >= cl->list.size())
  {
    if(m_parent != nullptr)
    {
      throw file_format_error("List size is below " + std::to_string(a), m_parent->filePath(), m_parent->c_data(), m_offset );
    }
    else
    {
      throw chunk_format_error("List size is below " + std::to_string(a), m_offset );
    }
  }
  return *cl->list[a];
}

std::ostream& operator<<(std::ostream& stream, Chunk const& a)
{
  stream << a.strval();
  return stream;
}

DataChunk::~DataChunk()
{
  for(auto it : values)
  {
    if(it.second != nullptr)
    delete it.second;
  }
}

ChunkList::~ChunkList()
{
  for(auto it : list)
  {
    if(it!=nullptr)
    delete it;
  }
}

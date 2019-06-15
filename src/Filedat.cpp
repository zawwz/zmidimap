#include "Filedat.hpp"

#include <exception>

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
  if(m_dataChunk!=nullptr)
  {
    delete m_dataChunk;
  }
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

bool Filedat::importFile()
{
  std::ifstream stream(m_filePath);
  if(!stream)
    return false;

  std::string str, line;
  while(stream)
  {
    getline(stream, line);
    str += (line + '\n');
  }
  this->clear();
  m_dataChunk = new Chunk(str);
  return true;
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
  stream << this->strval();
  return true;
}

std::string Filedat::strval() const
{
  if(m_dataChunk == nullptr)
    return "";
  else
    return m_dataChunk->strval();
}

void Chunk::set(Chunk const& in)
{
  this->set(in.strval());
}

void Chunk::set(std::string const& in)
{
  this->clear();
  std::string str=ridUnread(in);
  if(str[0]=='[')
  {
    std::vector<std::string> vstr = readList(in);
    ChunkList* cl = new ChunkList();
    for(auto it : vstr)
    {
      cl->list.push_back(new Chunk(it));
    }
    m_achunk=cl;
  }
  else if(str[0]=='{')
  {
    str=decapsulate(str);
    std::pair<std::string, std::string> pstr;
    int ti=0;
    DataChunk* dc = new DataChunk();
    while(str.size()>0 && ti>=0)
    {
      pstr = readValue(str,0,&ti);
      if(ti>=0)
      {
        dc->values.insert(std::make_pair(pstr.first , new Chunk(pstr.second)));
        if(ti > (int) str.size())
          throw std::runtime_error("Wrong file format at:\n" + str);
        str=str.substr(ti,str.size()-ti);
      }
    }
    m_achunk=dc;
  }
  else
  {
    DataVal* cv = new DataVal();
    cv->val = in;
    m_achunk=cv;
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
    this->addToList(it);
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
      ret += repeatString(aligner,alignment+1);
      ret += it.first;
      ret += '=';
      if(it.second!=nullptr)
        ret += it.second->strval(alignment+1, aligner);
      ret += '\n';
    }
    ret += repeatString(aligner, alignment);
    ret += '}';
    return ret;
  }
  else if(this->type()==AbstractChunk::list)
  {
    ChunkList* lp = dynamic_cast<ChunkList*>(m_achunk);
    std::string ret="[\n";
    for(auto it : lp->list)
    {
      ret += repeatString(aligner, alignment+1);
      if(it!=nullptr)
        ret += it->strval(alignment+1, aligner);
      ret += ",\n";
    }
    ret.erase(ret.end()-2);
    ret += repeatString(aligner, alignment);
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
    throw std::runtime_error("Chunk isn't a {}:" + this->strval());
  DataChunk* dc = dynamic_cast<DataChunk*>(m_achunk);
  auto fi = dc->values.find(in);
  if(fi == dc->values.end())
    throw std::runtime_error("Chunk doesn't have '" + in + "' flag:\n" + this->strval());
  return *fi->second;
}

Chunk& Chunk::subChunkRef(unsigned int a) const
{
  if(this->type()!=AbstractChunk::list)
    throw std::runtime_error("Chunk isn't a []:" + this->strval());
  ChunkList* cl = dynamic_cast<ChunkList*>(m_achunk);
  if(a >= cl->list.size())
    throw std::runtime_error("List size is below " + std::to_string(a) + ":\n" + this->strval());
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

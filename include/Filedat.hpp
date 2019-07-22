#ifndef FILEDAT_H
#define FILEDAT_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#include <cstring>

class file_format_error : public std::exception
{
public:
  file_format_error(const std::string& what, const std::string& origin, const char* data, int where)  { desc=what; index=where; filename=origin; cdat=data; }

  const char * what () const throw () {return desc.c_str();}
  const int where () const throw () {return index;}
  const char * data() const throw () {return cdat;}
  const char * origin() const throw () {return filename.c_str();}
private:
  std::string desc;
  int index;
  std::string filename;
  const char* cdat;
};

class chunk_format_error : public std::exception
{
public:
  chunk_format_error(std::string const& what, int where)  { desc=what; index=where; }

  const char * what () const throw () {return desc.c_str();}
  const int where () const throw () {return index;}
private:
  std::string desc;
  int index;
};

void printFileException(file_format_error& exc);
void printErrorIndex(const char* in, const int index, const std::string& message, const std::string& origin);

class Filedat;

class AbstractChunk
{
public:
  enum typeEnum { none, val, chunk, list};

  typeEnum type() { return m_type; }

  AbstractChunk() { m_type=AbstractChunk::none; }
  virtual ~AbstractChunk() {}

protected:
  typeEnum m_type;
};

class Chunk
{
public:
  Chunk(const char* in, const int in_size,  int offset=0, Filedat* data=nullptr)
    { m_achunk=nullptr; set(in, in_size, offset, data);}
  Chunk(std::string const& in,              int offset=0, Filedat* data=nullptr)
    { m_achunk=nullptr; set(in, offset, data);}

  Chunk(Chunk const& in) { m_achunk=nullptr; set(in);}

  void clear() { if(m_achunk!=nullptr) delete m_achunk; m_achunk=nullptr; }
  ~Chunk() { clear(); }

  Filedat* parent() const { return m_parent; }
  int offset() const { return m_offset; }


  void set(const char* in, const int in_size, int offset=0, Filedat* data=nullptr);
  void set(std::string const& in, int offset=0, Filedat* data=nullptr) { this->set(in.c_str(), in.size(), offset, data); }

  void set(Chunk const& in) { this->set(in.strval(), in.offset(), in.parent()); } // TODO

  std::string strval(unsigned int alignment=0, std::string const& aligner="\t") const;


  // bool concatenate(Chunk const& chk); //concatenates chunks
  bool addToChunk(std::string const& name, Chunk const& val); //adds if datachunk
  bool addToChunk(std::vector<std::pair<std::string, Chunk>> const& vec); //adds if datachunk
  bool addToList(Chunk const& val); //adds if list
  bool addToList(std::vector<Chunk> const& vec); //adds if list
  inline bool add(std::string const& name, Chunk const& val) { return addToChunk(name, val); } //adds if datachunk
  inline bool add(std::pair<std::string, Chunk> const& pair) { return add(pair.first, pair.second); } //adds if datachunk
  inline bool add(std::vector<std::pair<std::string, Chunk>> const& vec) { return addToChunk(vec); } //adds if datachunk
  inline bool add(Chunk const& val) { return addToList(val); } //adds if list
  inline bool add(std::vector<Chunk> const& vec) { return addToList(vec); } //adds if list

  Chunk copy() const { return Chunk(*this); }
  Chunk* pcopy() const { return new Chunk(*this); }

  AbstractChunk* getp() const { return m_achunk; }
  AbstractChunk::typeEnum type() const { if(m_achunk!=nullptr) return m_achunk->type(); else return AbstractChunk::none; }
  int listSize() const;

  Chunk* subChunkPtr(std::string const& a) const; //datachunk
  Chunk* subChunkPtr(unsigned int a) const; //chunklist
  Chunk& subChunkRef(std::string const& a) const; //datachunk
  Chunk& subChunkRef(unsigned int a) const; //chunklist

  Chunk& operator[](std::string const& a) const { return subChunkRef(a); }
  Chunk& operator[](unsigned int a) const { return subChunkRef(a); }
  Chunk& operator=(Chunk const& a) { set(a); return *this; }
  inline bool operator+=(std::pair<std::string, Chunk> const& a) { return add(a); }
  inline bool operator+=(std::vector<std::pair<std::string, Chunk>> const& a) { return add(a); }
  inline bool operator+=(Chunk const& a) { return add(a); }
  inline bool operator+=(std::vector<Chunk> const& a) { return add(a); }
  // inline bool operator*=(Chunk const& a) { concatenate(a); }

  //add operator+ and operator*


protected:
  Filedat* m_parent;
  int m_offset;

  AbstractChunk* m_achunk;
};

std::ostream& operator<<(std::ostream& stream, Chunk const& a);

class DataVal : public AbstractChunk
{
public:
  DataVal() { m_type=AbstractChunk::val; }
  virtual ~DataVal() {}

  std::string val;
};

class DataChunk : public AbstractChunk
{
public:
  DataChunk() { m_type=AbstractChunk::chunk; }
  virtual ~DataChunk();

  std::map<std::string, Chunk*> values;

};

class ChunkList : public AbstractChunk
{
public:
  ChunkList() { m_type=AbstractChunk::list; }
  virtual ~ChunkList();

  std::vector<Chunk*> list;
};

class Filedat
{
public:
  Filedat();
  Filedat(std::string const& in);
  virtual ~Filedat();

  bool readTest() const;

  void importFile();
  bool exportFile(std::string const& path="", std::string const& aligner="\t") const;

  void clear();

  inline std::string filePath() const { return m_filePath; }
  inline void setFilePath(std::string const& in) { m_filePath=in; }

  std::string strval(std::string const& aligner="\t") const;

  Chunk* pchunk() const { return m_dataChunk; }
  Chunk& chunk() const { return *m_dataChunk; }

  inline Chunk* pdata() const { return m_dataChunk; }
  inline Chunk& data() const { return *m_dataChunk; }

  inline const std::string& stringdata() const { return m_data; }
  inline const char* c_data() const { return m_data.c_str(); }

  Chunk& operator[](const std::string& index)
  {
    return m_dataChunk->subChunkRef(index);
  }
  Chunk& operator[](const int index)
  {
    return m_dataChunk->subChunkRef(index);
  }

private:
  std::string m_filePath;
  std::string m_data;
  Chunk* m_dataChunk;
};

#endif //FILEDAT_H

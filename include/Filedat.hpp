#ifndef FILEDAT_H
#define FILEDAT_H

#include "stringTools.hpp"

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

//TESTS NEEDED

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
  Chunk() { m_achunk=nullptr; }
  Chunk(char* const in) { m_achunk=nullptr; set(std::string(in)); }
  Chunk(std::string const& in) { m_achunk=nullptr; set(in); }
  Chunk(Chunk const& in) { m_achunk=nullptr; set(in); }
  void clear() { if(m_achunk!=nullptr) delete m_achunk; m_achunk=nullptr; }
  ~Chunk() { clear(); }

  void set(std::string const& in);
  void set(Chunk const& in); //TO OPTIMIZE
  std::string strval(unsigned int alignment=0, std::string const& aligner="\t") const;

  // bool concatenate(Chunk const& chk); //concatenates chunks
  bool addToChunk(std::string const& name, Chunk const& val); //adds if datachunk
  inline bool addToChunk(std::pair<std::string, Chunk> const& pair) { return add(pair.first, pair.second); } //adds if datachunk
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
  inline bool operator+=(std::pair<std::string, Chunk> const& a) { return addToChunk(a); }
  inline bool operator+=(std::vector<std::pair<std::string, Chunk>> const& a) { return addToChunk(a); }
  inline bool operator+=(Chunk const& a) { return addToList(a); }
  inline bool operator+=(std::vector<Chunk> const& a) { return addToList(a); }
  // inline bool operator*=(Chunk const& a) { concatenate(a); }

  //add operator+ and operator*

protected:
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

  bool importFile();
  bool exportFile(std::string const& path="", std::string const& aligner="\t") const;

  void clear();

  std::string filePath() const { return m_filePath; }
  void setFilePath(std::string const& in) { m_filePath=in; }

  std::string strval() const;

  inline Chunk* pchunk() const { return m_dataChunk; }
  inline Chunk& chunk() const { return *m_dataChunk; }

  inline Chunk* pdata() const { return m_dataChunk; }
  inline Chunk& data() const { return *m_dataChunk; }

private:
  std::string m_filePath;
  Chunk* m_dataChunk;
};

#endif //FILEDAT_H

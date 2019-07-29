#ifndef FILEDAT_H
#define FILEDAT_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#include <cstring>

class format_error : public std::exception
{
public:
  format_error(const std::string& what, const std::string& origin, const std::string& data, int where)  { desc=what; index=where; filename=origin; sdat=data; }

  const char * what () const throw () {return desc.c_str();}
  const int where () const throw () {return index;}
  const char * data() const throw () {return sdat.c_str();}
  const char * origin() const throw () {return filename.c_str();}
private:
  std::string desc;
  int index;
  std::string filename;
  std::string sdat;
};

void printErrorIndex(const char* in, const int index, const std::string& message, const std::string& origin);
inline void printFormatException(format_error& exc) { printErrorIndex(exc.data(), exc.where(), exc.what(), exc.origin()); }

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
  Chunk() { m_achunk=nullptr; }

  Chunk(const char* in)
    { m_achunk=nullptr; set(in, strlen(in), 0, nullptr); }
  Chunk(std::string const& in)
    { m_achunk=nullptr; set(in, 0, nullptr); }
  Chunk(const char* in, const int in_size,  int offset=0, Filedat* data=nullptr)
    { m_achunk=nullptr; set(in, in_size, offset, data); }
  Chunk(std::string const& in,              int offset=0, Filedat* data=nullptr)
    { m_achunk=nullptr; set(in, offset, data); }

  Chunk(Chunk const& in) { m_achunk=nullptr; set(in); }

  void clear() { if(m_achunk!=nullptr) delete m_achunk; m_achunk=nullptr; }
  ~Chunk() { clear(); }

  Filedat* parent() const { return m_parent; }
  int offset() const { return m_offset; }

  void set(const char* in, const int in_size, int offset=0, Filedat* data=nullptr);
  void set(std::string const& in, int offset=0, Filedat* data=nullptr) { this->set(in.c_str(), in.size(), offset, data); }

  void set(Chunk const& in) { this->set(in.strval(), in.offset(), in.parent()); } // TODO

  std::string strval(unsigned int alignment=0, std::string const& aligner="\t") const;


  void addToChunk(std::string const& name, Chunk const& val); //adds if datachunk
  void addToChunk(std::vector<std::pair<std::string, Chunk>> const& vec); //adds if datachunk
  void addToList(Chunk const& val); //adds if list
  void addToList(std::vector<Chunk> const& vec); //adds if list
  inline void add(std::string const& name, Chunk const& val) { addToChunk(name, val); } //adds if datachunk
  inline void add(std::pair<std::string, Chunk> const& pair) { add(pair.first, pair.second); } //adds if datachunk
  inline void add(std::vector<std::pair<std::string, Chunk>> const& vec) { addToChunk(vec); } //adds if datachunk
  inline void add(Chunk const& val) { addToList(val); } //adds if list
  inline void add(std::vector<Chunk> const& vec) { addToList(vec); } //adds if list
  // void concatenate(Chunk const& chk); //concatenates chunks

  Chunk copy() const { return Chunk(*this); }
  Chunk* pcopy() const { return new Chunk(*this); }

  AbstractChunk* getp() const { return m_achunk; }
  AbstractChunk::typeEnum type() const { if(m_achunk!=nullptr) return m_achunk->type(); else return AbstractChunk::none; }
  int listSize() const;

  Chunk* subChunkPtr(std::string const& a) const; //datachunk
  Chunk* subChunkPtr(unsigned int a) const; //chunklist
  Chunk& subChunkRef(std::string const& a) const; //datachunk
  Chunk& subChunkRef(unsigned int a) const; //chunklist

  Chunk& operator[](std::string const& a) const                                 { return subChunkRef(a); }
  Chunk& operator[](unsigned int a) const                                       { return subChunkRef(a); }
  Chunk& operator=(Chunk const& a)                                              { set(a); return *this; }
  inline Chunk& operator+=(std::pair<std::string, Chunk> const& a)              { add(a); return *this; }
  inline Chunk& operator+=(std::vector<std::pair<std::string, Chunk>> const& a) { add(a); return *this; }
  inline Chunk& operator+=(Chunk const& a)                                      { add(a); return *this; }
  inline Chunk& operator+=(std::vector<Chunk> const& a)                         { add(a); return *this; }
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

  void import_file(const std::string& path="");
  void import_stdin();
  void import_string(const std::string& data);
  bool export_file(std::string const& path="", std::string const& aligner="\t") const;

  void clear();

  inline std::string filePath() const { return m_filePath; }
  inline void setFilePath(std::string const& in) { m_filePath=in; }

  std::string strval(std::string const& aligner="\t") const;

  inline Chunk* pchunk() const { return m_dataChunk; }
  inline Chunk& chunk() const { return *m_dataChunk; }

  inline Chunk* pdata() const { return m_dataChunk; }
  inline Chunk& data() const { return *m_dataChunk; }

  inline const std::string& stringdata() const { return m_data; }
  inline const char* c_data() const { return m_data.c_str(); }

  inline Chunk& operator[](const std::string& index) { return m_dataChunk->subChunkRef(index); }
  inline Chunk& operator[](const int index) { return m_dataChunk->subChunkRef(index); }

private:
  //functions
  void generateChunk();
  //attributes
  std::string m_filePath;
  std::string m_data;
  Chunk* m_dataChunk;
};

#endif //FILEDAT_H

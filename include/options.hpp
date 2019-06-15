#ifndef OPTIONS_H
#define OPTIONS_H

//DUPLICATES NOT HANDLED: takes last one
//NO ORDER: no way to know options order
//RETURNS TRANSITIONAL STATE IF ERROR

#include <vector>
#include <string>
#include <utility>

std::vector<std::string> argVector(int argc, char** argv);

class Option
{
public:
  Option();
  Option(char c, bool arg);
  Option(std::string const& str, bool arg);
  Option(char c, std::string const& str, bool arg);
  virtual ~Option();

  bool shortDef;
  char charName;

  bool longDef;
  std::string strName;

  bool activated;

  bool takesArgument;
  std::string argument;
};

class OptionSet
{
public:
  OptionSet();
  virtual ~OptionSet();

  void addOption(Option opt) { m_options.push_back(opt); }

  Option* findOption(char c);
  Option* findOption(std::string const& str);

  std::ostream* errStream;

  std::pair<std::vector<std::string>,bool> getOptions(std::vector<std::string> input);

private:
  std::vector<Option> m_options;

};

#endif //OPTIONS_H

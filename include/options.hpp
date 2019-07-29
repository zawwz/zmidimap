#ifndef OPTIONS_H
#define OPTIONS_H

//DUPLICATES NOT HANDLED: takes last one
//NO ORDER: no way to know options order
//RETURNS TRANSITIONAL STATE IF ERROR

#include <vector>
#include <string>
#include <utility>


// Turn argc/argv into a vector<string>
std::vector<std::string> argVector(int argc, char** argv);

class Option
{
public:
  /* CTOR/DTOR */
  //ctors
  Option();
  Option(char c, bool arg, std::string helptext="", std::string argname="arg");
  Option(std::string const& str, bool arg, std::string helptext="", std::string argname="arg");
  Option(char c, std::string const& str, bool arg, std::string helptext="", std::string argname="arg");
  //dtors
  virtual ~Option();

  /* FUNCTIONS */

  // Print command help. Puts leftpad spaces before printing, and rightpad space until help
  void printHelp(int leftpad, int rightpad);

  /* PROPERTIES */

  bool shortDef;  // has a char definition
  char charName;

  bool longDef;   // has a string definition
  std::string strName;

  bool takesArgument;     // option takes an argument

  std::string help_text;  // text to display in printHelp
  std::string arg_name;   // name of the argument to display in printHelp

  /* PROCESSING STATUS */

  bool activated;         // option was activated

  std::string argument;   // argument of the option

};

class OptionSet
{
public:
  /* CTOR/DTOR */
  OptionSet();
  virtual ~OptionSet();

  /* PROPERTIES */

  // Stream on which errors are sent. Default stderr
  std::ostream* errStream;

  /* FUNCTIONS */

  /*CREATION FUNCTIONS*/
  //Adding an option. Refer to Option ctors
  void addOption(Option opt) { m_options.push_back(opt); }

  /*PRINT FUNCTIONS*/
  // Print command help. Puts leftpad spaces before each line, and rightpad space until help
  void printHelp(int leftpad=2, int rightpad=25);

  /*QUERY FUNCTIONS*/
  // Find an option with its charname
  Option* findOption(char c);
  // Find an option with its stringname
  Option* findOption(std::string const& str);

  /*PROCESSING FUNCTIONS*/
  // Process through options.
  // pair.first  : vector with arguments that were not identified as options
  // pair.second : bool indicating status. True if no error encountered, false if errors
  std::pair<std::vector<std::string>,bool> getOptions(std::vector<std::string> input);
  std::pair<std::vector<std::string>,bool> getOptions(int argc, char** argv) { return getOptions(argVector(argc, argv)); }

private:
  std::vector<Option> m_options;

};

#endif //OPTIONS_H

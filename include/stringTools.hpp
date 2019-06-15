#ifndef READ_H
#define READ_H

#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <stdint.h>

//NOTE: all functions here are probably not the most efficient

/*Translates string of 1 to 2 hex char into a byte
returns 0 if an error occured
*/
uint8_t hexStringToByte(std::string const& in);

/*Translates an hex digit to its corresponding value
*/
int8_t hexDigitToNum(char car);

/*Translates a byte to a two char hex digit string
*/
std::string byteToHexString(uint8_t const& byte);

/*Translates a number into its hex char
only answers correctly if 0<=num<16
otherwise answers \0
*/
char toHexDigit(uint8_t const& num);

/*Reapeats n times string a
*/
std::string repeatString(std::string const& a, unsigned int n);

/*Repeats n times char a
*/
std::string repeatChar(char a, unsigned int n);

/*Returns true if it an alpha char
*/
bool isAlpha(char a);

/*Returns true if it a number char
*/
bool isNum(char a);

/*Returns true if it an hexadecimal char
*/
bool isHexdec(char a);

/*Checks if two strings are insensitively case equal
*/
bool equalNoCase(std::string const& st1, std::string const& st2);

/*Returns true if str1 and str2 have a common char
*/
bool hasCommonChar(std::string const& str1, std::string const& str2);

/*Fuses elements of a string vector, from [first] to [last] adding <separator> between each
If separator = \0 , adds nothing
if last < 0 , goes until the end
*/
std::string fuse(std::vector<std::string> const& vec, int first, int last=-1, char separator=' ');

/*Finds rank of char in string
returns -1 if not found
*/
int findInString(std::string const& in, char const find);

/*Returns the number of times char has appeared in string
*/
unsigned int charCount(std::string const& in, char const find);

/*************************************/

bool isRead(char in); //Read characters (ascii-7bit) for following functions

/*Decapsulates the content of string
If first read char is encapsulator, reads until decapsulator
If not, just gets rid of unread char at beginning and end
Encapsulators: "",{},[],<>
*/
std::string decapsulate(std::string const& in);

/*Removes unread chars at beggining and end of the string
*/
std::string ridUnread(std::string const& in);

/*Reads a variable of type string in a string.
It's important to place the container characters right after the variable name
ex: "variable={ text }"
Smart reading, if another { is detected, it will wait for another } before ending
Considers \n as the separator if there arent {}
Reads only ascii-7bit
*/
std::string readInString(std::string const& in, std::string const& name, unsigned int occurence=0);

/*Same as above, but returns pair of value name and value itself of occurence given
instead of value name
puts beginning of value in rank if rank != nullptr
*/
std::pair<std::string, std::string> readValue(std::string const& in, unsigned int occurence=0, int* rank=nullptr);

/*Reads list in format [  ,  ,  ] and splits into vector
*/
std::vector<std::string> readList(std::string const& in);

/*Splits string using given delimiter. Uses space if not specified
Between encapsulator and decapsulator: delims added
INVALID DELIM: \ <encapsulator> <decapsulator>
If delim is invalid, returns empty vector
Does not include encapsulators in output
\<char> adds \ and char, even if it is an encapsulator
*/
std::vector<std::string> splitString(std::string const& in, char const delim=' ', char encapsulator='\"', char decapsulator=0);

/*Replaces char f by r
Returns number of char replaced
*/
int replaceChar(std::string& in, char const f,char const r);

/*Returns true if char c is in str
*/
bool isIn(char const c, std::string const& str);

/*Compacts suites of any chars from string chars, into char into
note: equivalent to   tr -s <chars> <into>   in bash
*/
std::string encompact(std::string const& in, std::string const& chars=" \n\r\t\b", char const into=' ');

#endif // READ_H

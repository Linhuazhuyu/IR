#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#define DIR_SIZE 10000

const char _WHITESPACE[] = " \t\n\x0b\x0c\r\0";
const std::string WHITESPACE(_WHITESPACE,sizeof(_WHITESPACE));

// convert from string to a ginven numeric type
template <class T>
bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}


// Converts int to string
std::string itos(int i);	

unsigned int myPow(unsigned int base, unsigned int exp);

std::string termDir(unsigned int termId);

std::string docDir(unsigned int docId);

// returns a binary file length
unsigned long int fileLength(std::ifstream& file);

/**Splits a string into pieces using separator as separator.
 *
 * @param inString The string to be splited
 * @param separator The string used as delimiter.
 * @param maxsplit If non-zero, at most maxsplit splits are done.
 * 		   Defaults to 0, i.e., there is no limit of splits.
 *
 */
std::vector<std::string>
split (const std::string &inString, const std::string &separator,
		unsigned int maxsplit = 0);

std::string& to_lower(std::string& s);

std::string strip(const std::string s);

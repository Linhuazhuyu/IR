#include "utils.h"
#include <sstream>
#include <algorithm>

/*
 * Useful little things to have around...
 */

// Converts int to string
std::string itos(int i){	
   std::stringstream s;
   s << i;
   return s.str();
}

// Created my own pow function because the cmath one
// pisses me off to no end.
unsigned int myPow(unsigned int base, unsigned int exp) {
   if (exp == 0) {
      return 1;
   } else {
      unsigned int acc = 1;
      for (int i = 0; i < exp; ++i){
         acc*=base;
      }
      return acc;
   }
}

// Returns the dir where a given term should be
std::string termDir(unsigned int termId){
   div_t dir;
   dir = div(termId,DIR_SIZE);
   return "voc/" + itos(dir.quot) + "/" + itos(termId);
}

// Returns the dir where a given term should be
std::string docDir(unsigned int docId){
   div_t dir;
   dir = div(docId,DIR_SIZE);
   return "../colector/collected-bk/" + itos(dir.quot) + "/" 
      + itos(docId) + ".gz";
}

// returns a binary file length
unsigned long int fileLength(std::ifstream& file) {
   file.seekg (0, std::ios::end);
   unsigned long int l = file.tellg();
   file.seekg (0, std::ios::beg);
   return l;
}

std::string& to_lower(std::string& s) {
	std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))tolower);
	return s;
}

std::string strip(const std::string _s) {
	std::string::size_type start;
	std::string::size_type end;

	std::string s = _s; // STFU!

	// take spaces out of the start...
	start = s.find_first_not_of(WHITESPACE);
	if (start == std::string::npos) {
		// String made of spaces...
		s.clear();
		return s;
	}
	s = s.substr(start);

	// take space out of the end of the string
	end = s.find_last_not_of(WHITESPACE);
	if (end == std::string::npos) {
		// Segurança morreu de seguro...
		// WE SHOULD NOT BE HERE but...
		// String made of spaces...
		s.clear();
		return s;
	}
	s = s.substr(0,end +1);
	return s;
}

std::vector<std::string>
split (const std::string &inString, const std::string &separator,
		unsigned int maxsplit)
{
	std::vector<std::string> returnVector;
	std::string::size_type start = 0;
	std::string::size_type end = 0;
	unsigned int count = maxsplit;

	while ( ((end=inString.find (separator, start)) != std::string::npos) &&
			(maxsplit ? count-- : true) )
	{
		returnVector.push_back (inString.substr (start, end-start));
		start = end+separator.size();
	}

	returnVector.push_back (inString.substr (start));

	return returnVector;

}

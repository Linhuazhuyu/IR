#include <string>
#include <ext/hash_map>
#include <queue>
#include <vector>
#include "myFileHandler.h"
#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto
#include "boost/filesystem/path.hpp"       // ditto

#define RUN_SIZE 2000000 // Got this number from thin air, but...
// Maximum number of documents in a folder
#define DIR_SIZE 10000
// The last docId we have
#define LAST_DOC 1244550
// Whether or not to comprees the gaps in the final index
#define COMPRESS 0

namespace stdext=__gnu_cxx;
namespace bfs=boost::filesystem;

// For the hash
struct eqstr{
  bool operator()(const std::string& s1, const std::string& s2) const{
    return s1 == s2;
  }
};

struct myhashfunc{
   stdext::hash<const char*> H;
   size_t operator()(const std::string& s1) const{
      return H(s1.c_str());
   }
};

class mycomparison{
   public:
      bool operator() (const RunOcurrence& ro1, const RunOcurrence& ro2) const;
};

// Necessary to allow a heap of RunOcurrence
bool mycomparison::operator() (const RunOcurrence& ro1, 
      const RunOcurrence& ro2) const {
   return !(ro1 < ro2);
}


// My typedefs
typedef stdext::hash_map<std::string, unsigned int, 
        myhashfunc, eqstr> vocabulary;

typedef stdext::hash_map<unsigned int, Ocurrences> localVocabulary;

typedef std::priority_queue<RunOcurrence,std::vector<RunOcurrence>
         ,mycomparison> myHeap;

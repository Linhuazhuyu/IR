#include <string>
#include <queue>
#include <ext/hash_set>
#include <vector>

namespace stdext=__gnu_cxx;

class MyException: public std::exception{};

// Converts int to string
std::string itos(int);

// For the domain priority queue
class Domain{
   public:
      Domain();
      Domain(std::string, time_t, std::queue<std::string>*);
      std::string domainName;
      time_t timestamp;
      std::queue<std::string> *pageQueue;
      std::vector<std::string> robotsDisallow;
      bool robotsPermit(std::string);
      bool hasRobots;
      void addRobots(std::vector<std::string>);
};

// For the Hash Set
class PageCounter{
   public:
      PageCounter();
      PageCounter(std::string, unsigned int);
      void addPageCount();
      std::string print();
      std::string url;
      unsigned int counter;
};

class mycomparison{
   public:
      bool operator() (const Domain& d1, const Domain& d2) const;
};

bool operator<(const PageCounter& pc1, const PageCounter& pc2);

struct eqpage{
  bool operator()(const PageCounter& pc1, const PageCounter& pc2) const{
    return pc1.url == pc2.url;
  }
};

struct myhashfunc2{
   stdext::hash<const char*> H;
   size_t operator()(const PageCounter& pc1) const{
      return H(pc1.url.c_str());
   }
};

struct PageStruct {
  char *memory;
  size_t size;
  std::string myUrl;
};

std::string trim (std::string);

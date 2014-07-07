#include <ext/hash_map>
#include <string>

namespace stdext=__gnu_cxx;

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

typedef stdext::hash_map<std::string, unsigned int, myhashfunc, eqstr> myHash;

class Entities {
   private:
   myHash entityHash;
   // Fills the entity hash
   void initEntityHash();
   std::string convertUnicode(std::string);

   public:
   Entities();
   ~Entities();
   // Returns the converted entity
   std::string convertEntity(std::string);
};

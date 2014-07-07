#include <string>
#include <set>
#include "zfilebuf.h"

struct PageStruct;

namespace parser{
   std::set<std::string> getLinks(memory_buffer page, std::string base);
   std::string domainName(std::string url);
}

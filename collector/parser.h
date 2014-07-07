#include <string>
#include <set>
//#include "DataTypes.h"

struct PageStruct;

namespace parser{
   std::set<std::string> getLinks(PageStruct page, std::string base);
   std::string domainName(std::string url);
}

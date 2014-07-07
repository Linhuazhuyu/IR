#include <string>
//#include "DataTypes.h"

struct PageStruct;

namespace mycurl{
   void curlInit();
   PageStruct getPage(const std::string);
   void clearChunk(PageStruct);
}

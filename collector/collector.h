#include <string>

// Maximum number of documents in a folder
#define DIR_SIZE 10000
// Time (in secs) we should wait before asking the same server
#define WAIT_BETWEEN_REQS 30
// Number of active threads
#define NUM_THREADS 50
// Maximum size of the known url set
#define MAX_SET_SIZE 2500000
// Maximum number of pages in the queues
#define MAX_URLS_IN_QUEUE 2500000

namespace collector{
   std::string getDocID();
   void insertUrl(std::string url);
   std::string getUrl();
   unsigned long int viewDocID();
   void saveState();
}

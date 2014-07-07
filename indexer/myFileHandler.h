#include <string>
#include <fstream>
#include <vector>
#include "ocurrences.h"
#include <queue>

class RunFile {
   public:
      RunFile(std::string, unsigned int);
      ~RunFile();
      bool getRecord(Ocurrences&);
   private:
      bool fillBuffer();
      std::string filename;
      std::ifstream fh;
      std::streampos currentPos;
      unsigned int buffersize;
      unsigned long int length;
      std::queue<Ocurrences> buffer;
      void openFile();
      void closeFile();
      void getFileSize();
};

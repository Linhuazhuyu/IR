#include "myFileHandler.h"
#include <iostream>
#include "utils.h"

using namespace std;

void RunFile::getFileSize() {
   openFile();
   fh.seekg (0, ios::end);
   length = fh.tellg();
   fh.seekg (0, ios::beg);
   closeFile();
}

void RunFile::openFile() {
   fh.open(filename.c_str(), ios::binary);
}

void RunFile::closeFile() {
   fh.close();
}

RunFile::RunFile(string file, unsigned int bsize){
   filename = file;
   buffersize = bsize;
   currentPos = 0;
   getFileSize();
}

RunFile::~RunFile(){
}

bool RunFile::fillBuffer() {
   //cout << "Filling from run " << filename << endl;
   bool readsome = false;
   openFile();
   fh.seekg(currentPos);
   Ocurrences* o;
   unsigned int bufsize = buffer.size();
   while ((currentPos < length) && (bufsize < buffersize)){
      readsome = true;
      o = new Ocurrences();
      fh.read ((char*)o, sizeof(Ocurrences));
      buffer.push(*o);
      delete o;
      currentPos = fh.tellg();
      ++bufsize;
   }
   //cout << "Closing run " << filename << endl;
   closeFile();
   return readsome;
}

bool RunFile::getRecord(Ocurrences& o) {
   if (buffer.size() > 0) {
      o = buffer.front();
      buffer.pop();
      return true;
   } else {
      if (fillBuffer()) {
         o = buffer.front();
         buffer.pop();
         return true;
      } else {
         return false;
      }
   }
}

/*
// FIXME FOR TESTING ONLY, DAMMIT!! 
void storeRun(int runId, Ocurrences o) {
   // creating the output file
   string outName = "runs/run"+itos(runId);
   ofstream outFile(outName.c_str(), ios::binary | ios::app);
   outFile.write ((char*)&o, sizeof (Ocurrences));
   outFile.close();
}


int main() {
   Ocurrences o;
   RunFile* rf[3];
   storeRun(0,Ocurrences(9,0,1));
   storeRun(0,Ocurrences(8,1,2));
   storeRun(0,Ocurrences(7,2,3));
   storeRun(1,Ocurrences(9,11,4));
   storeRun(1,Ocurrences(8,22,5));
   storeRun(1,Ocurrences(7,33,6));
   storeRun(2,Ocurrences(9,111,7));
   storeRun(2,Ocurrences(8,222,8));
   storeRun(2,Ocurrences(7,333,9));
   for (int i = 0; i < 3; ++i) {
      rf[i] = new RunFile("runs/run"+itos(i),2);
   }
   for (int i = 0; i < 5; ++i){
      cout << "Elementos na posição " << i << endl;
      for (int j = 0; j < 3; ++j) {
         if (rf[j]->getRecord(o)) {
            cout << "Run " << j << "--> ";
            cout << o.term << ";" << o.document << ";" << o.frequency << endl; 
         } else cout << "No dogs allowed...\n";
      }
   }
}
*/

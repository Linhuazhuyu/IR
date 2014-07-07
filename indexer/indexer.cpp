#include <string>
#include <iostream>
#include "indexer.h"
#include "zfilebuf.h"
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <queue>
#include "utils.h"
#include <set>
#include "egamma.h"
#include <math.h>
#include "vector_model.h"
#include "httpserver.h"
#include <sys/sendfile.h>
#include <stdio.h>
#include <sys/stat.h>

using namespace std;

// My globals, just to make life less troublesome
unsigned int docId = 0;
vocabulary voc;
unsigned int wordId = 0;
unsigned int elements_in_run = 0;
unsigned int run_number = 0;
vector<Ocurrences>* run;
int currentDir = -1;
VectorModel vm;

// Prints all Ocurrences in a run. For testing purposes.
void printRun(ofstream& out) {
   vector<Ocurrences>::const_iterator it;
   for (it = run->begin(); it != run->end(); ++it) {
      out << "(" << it->term+1 << ";" << it->document <<
         ";" << it->frequency << ")\n";
   }
}

void printMerged(ofstream& out, tuple t){
   out << "(" << t.docId << ";" << t.freq << ")\n";
}

// Returns the location of the next file to process
string getNextFile(){
   if (docId > LAST_DOC) {
      return "";
   } else {
      div_t dir;
      dir = div(docId,DIR_SIZE);
      string tmp = "stripped/" + itos(dir.quot) 
         + "/" + itos(docId) + ".gz";
      //string tmp = "example/" + itos(docId) + ".gz";
      ++docId;
      return tmp;
   }
}

// Returns the dir where a new term should be written
string getTermDir(unsigned int termId){
   div_t dir;
   dir = div(termId,DIR_SIZE);
   if (dir.quot > currentDir) {
      string tmp("voc/"+itos(dir.quot));
      bfs::create_directory(bfs::path(tmp));
      ++currentDir;
   }
   return "voc/" + itos(dir.quot) + "/" + itos(termId);
}

unsigned int getNewId() {
   ++wordId;
   return (wordId-1);
}

unsigned int numRuns() {
   return run_number;
}

unsigned int getRunNumber() {
   ++run_number;
   return run_number-1;
}

// Index a given word
void index(string word, unsigned int docId, localVocabulary& lVoc) {
   unsigned int termId;
   if (voc.find(word) == voc.end()) {
      // New word!
      termId = getNewId();
      voc[word] = termId;
   } else {
      // Already known
      termId = voc[word];
   }
   // see if it has already appeared in this document
   if (lVoc.find(termId) == lVoc.end()) {
      Ocurrences o(termId,docId,1);
      lVoc[termId] = o;
   } else {
      Ocurrences o = lVoc[termId];
      ++o.frequency;
      lVoc[termId] = o;
   }
}

// Sorts and store a run, cleaning it afterwards
void storeRun() {
   // creating the output file
   unsigned int runId = getRunNumber();
   string outName = "runs/run"+itos(runId);
   ofstream outFile(outName.c_str(), ios::binary);
   // sorting the run
   sort(run->begin(), run->end());
   // Storing it
   vector<Ocurrences>::iterator it;
   Ocurrences o;
   for (it = run->begin(); it != run->end(); it++) {
      o = *it;
      // Storing for the calculus of the vector model
      vm.addTermFrequency(o.term);
      // end of vector model data gathering
      outFile.write ((char*)&o, sizeof (Ocurrences));
   }
   outFile.close();
   // Clean the run vector
   run->erase(run->begin(),run->end());
}

// Parse a given file
void parse(string file, unsigned int docId) {
   // decompressing the file
   memory_buffer dec(0,0);
	try{
		dec = decompres(file);
	} catch(...) {
	   // pass
	}
   // create  a local vocabulary
   localVocabulary lVoc;
   int i = 0;
   string token = "";
   // Will return a token, i. e., a string that will stop
   // at the first space found
   while (i < dec.second) {
      if (isspace(dec.first[i]) ||
            ispunct(dec.first[i])){
         // Consumed a token. Treat it accordingly
         if (token != "") {
            index(token, docId, lVoc);
         }
         token = "";
      } else {
         token += dec.first[i];
      }
      ++i;
   }
   // Document indexed. Cleaning it up
   delete[] dec.first;
   // Now to add the ocurrences to the run vector, 
   // until it's full
   localVocabulary::const_iterator it;
   for (it = lVoc.begin(); it != lVoc.end(); ++it) {
      if (run->size() < RUN_SIZE) {
         run->push_back(it->second);
      } else {
         storeRun();
         run->push_back(it->second);
      }
   }
   //delete lVoc;
}

// Merge the sorted runs
void mergeRuns() {
   // Everything looking good! Now to write the vocabulary
   ofstream outFile;
   /*
   vocabulary::iterator it;
   outFile.open("voc/vocabulary.txt");
   for (it = voc.begin(); it != voc.end(); ++it) {
      outFile << it->first << ";" << it->second << endl;
   }
   outFile.close();
   */
   // Preparing the accumulators for the vector model
   vm.startVecs(docId-1);
   // Create the heap
   myHeap mergeHeap;
   // Load the sorted runs using mmap
   vector<RunFile*> rfv;
   RunFile* rftmp;
   string filename;
   Ocurrences ocurr;
   RunOcurrence* ro;
   for (int i = 0; i < run_number; ++i) {
      filename = "runs/run" + itos(i);
      rfv.push_back(new RunFile(filename,RUN_SIZE/run_number));
      //rfv[0] = rftmp;
      // get the first element and put it in the heap
      if (!rfv[i]->getRecord(ocurr)) { 
         cout << "He should have got this record...\n";
         return;
      }
      ro = new RunOcurrence(ocurr,i);
      mergeHeap.push(*ro);
      delete ro;
   }
   unsigned int currentTerm = 0;
   unsigned int currentDoc = 0;
   unsigned int gap;
   tuple aux;
   outFile.open(getTermDir(currentTerm).c_str(), ios::binary);
   RunOcurrence first;
   while (not mergeHeap.empty()) {
      first = mergeHeap.top();
      mergeHeap.pop();
      // Mumbo jumbo
      // Accumulate data for the norms (vector model)
      vm.normAcc(first.document, first.term, first.frequency);
      // Get a new RunOcurrence from the same run and
      // put it in the heap, if possible
      if (rfv[first.run]->getRecord(ocurr)) {
         ro = new RunOcurrence(ocurr,first.run);
         mergeHeap.push(*ro);
         delete ro;
      }
      // Calculate the document gap
      if (first.term > currentTerm) {
         // New term, new gaps...
         currentDoc = 0;
      }
      gap = first.document - currentDoc;
      currentDoc = first.document;
      first.document = gap;
      // TODO Elias gamma compression will happend HERE
      // first.document = EliasGamma::Compress(first.document); //??
      // Saving to the file
      if (first.term > currentTerm) {
         outFile.close();
         currentTerm = first.term;
         outFile.open(getTermDir(currentTerm).c_str(), ios::binary);
      } else if (first.term < currentTerm) {
         cout << "Shouldn't be here...\n";
      }
      if (COMPRESS) {
         string tmp = eliasGammaCode(first.document);
         for (int i = 0; i < tmp.length(); ++i) {
            outFile << tmp[i];
         }
         outFile.write((char*)&first.frequency, sizeof(unsigned int));
      } else {
         aux.docId = first.document;
         aux.freq = first.frequency;
         outFile.write((char*)&aux, sizeof(tuple));
      }
   }
   outFile.close();
   // Finishing the norm calculation (vector model)
   vm.calculateNorms();
   for (int i = 0; i < run_number; ++i){
      delete rfv[i];
   }
}

// Puts the index back in memory
bool reloadIndex() {
   ifstream indexFile;
   indexFile.open("voc/vocabulary.txt");
   string entry;
   getline(indexFile,entry);
   while (!indexFile.eof()){
      unsigned int pos = entry.find(";");
      string word;
      word.assign(entry.c_str(),pos);
      entry.erase(0,pos+1);
      from_string<unsigned int>(pos, entry, std::dec);
      voc[word] = pos;
      getline (indexFile,entry);
   }
   return true;
}

// Put all the documents that have the desired term
// and return them as a set.
set<unsigned int> getResults(unsigned int termId) {
   unsigned int currentDoc = 0;
   set<unsigned int> results;
   ifstream termList;
   termList.open(getTermDir(termId).c_str(), ios::binary );
   // get the file size
   termList.seekg (0, ios::end);
   unsigned long int length = termList.tellg();
   termList.seekg (0, ios::beg);
   // Preparing to read the file
   tuple aux;
   //For elias gamma decoding only!   
   bool gotCode = false;
   int binbits; 
   unsigned char byte, aux2;
   string egamma = "";
   while (termList.tellg() < length) {
      if (COMPRESS) {
         binbits = 0;
         // Also for Elias Gamma decoding 
         while (not gotCode) {
            termList.read((char*)&byte, sizeof(unsigned char));
            if ((int)byte < 255) {
               // Elias gamma code ending here
               gotCode = true;
               // Must know how many unary bits are here to 
               // calculate the final number of binary bits
               // to read
               int lshift = 0;
               int unarybits = 0;
               aux2 = byte;
               aux2 << lshift;
               aux2 >> 7;
               while (aux2 & 00000001) {
                  ++unarybits;
                  ++binbits;
                  ++lshift;
                  aux2 = byte;
                  aux2 << lshift;
                  aux2 >> 7;
               }
               ++unarybits;
               // How many bits left for the binary code after this
               // byte is consumed
               binbits -= (8 - unarybits);
               // Accumulating the code bytes...
               egamma += byte;
               while (binbits > 0) {
                  termList.read((char*)&byte, sizeof(unsigned char));
                  egamma += byte;
                  binbits -=8;
               }
            } else {
               // Must keep track of the nunber of binary bits to get
               binbits+=8;
               // Accumulating the code bytes...
               egamma += byte;
            }
         }
         gotCode = false;
         aux.docId = eliasGammaDecode(egamma);
         egamma = "";
         termList.read((char*)&aux.freq, sizeof(unsigned int));
      } else {
         termList.read ((char*)&aux, sizeof(tuple));
         // TODO Elias Gamma decompression will happend here 
      }
      currentDoc += aux.docId;
      results.insert(currentDoc);
   }
   termList.close();
   return results;
}

void searchInIndex() {
   bool again = false;
   string opt;
   string query, term1, connective, term2;
   do {
      cout << "Enter a query (maximum 2 terms and 1 connective AND or OR):\n";
      getline(cin, query);
      if (query == "") {
         cout << "Try to write something next time, jackass...\n";
      } else {
         std::transform(query.begin(), query.end(), 
            query.begin(), (int(*)(int)) tolower);
         int pos = query.find(" ");
         if (pos == string::npos) {
            // one term query
            if (voc.find(query) == voc.end()) {
               cout << "No match, sorry.\n";
            } else {
               set<unsigned int> results = getResults(voc[query]);
               // Since there is only one term, print the results
               cout << "Documents that fit the query: ";
               set<unsigned int>::iterator it;
               for (it = results.begin(); it != results.end(); ++it){
                  cout << *it << " ";
               }
               cout << endl;
            }
         } else {
            // two term query
            set<unsigned int> resultsT1;
            set<unsigned int> resultsT2;
            // get the query terms and connective
            term1.assign(query.c_str(),pos);
            query.erase(0,pos+1);
            pos = query.find(" ");
            connective.assign(query.c_str(),pos);
            query.erase(0,pos+1);
            term2 = query;
            // Get the results
            if (voc.find(term1) != voc.end()) {
               resultsT1 = getResults(voc[term1]); 
            }
            if (voc.find(term2) != voc.end()) {
               resultsT2 = getResults(voc[term2]); 
            }
            // returns the result of the query
            if (connective == "and") {
               if (resultsT1.size() == 0 || resultsT1.size() == 0) {
                  cout << "No match.";
               } else {
                  set_intersection(resultsT1.begin(), resultsT1.end(), 
                     resultsT2.begin(), resultsT2.end(),
                     ostream_iterator<unsigned int>(cout, " "));
               }
            } else if (connective == "or") {
               if (resultsT1.size() == 0 && resultsT1.size() == 0) {
                  cout << "No match.";
               } else {
                  set_union(resultsT1.begin(), resultsT1.end(), 
                     resultsT2.begin(), resultsT2.end(),
                     ostream_iterator<unsigned int>(cout, " "));
               }
            } else {
               cout << "Unsupported connective";
            }
            cout << endl;
         }
      }
      cout << "Try again? (y/n) ";
      cin >> opt;
      if (opt == "y") {
         again = true;
      } else {
         again = false;
      }
   } while (again);      
}

void search() {
   // Setting up the http server 
   HttpServer server;
   std::vector<unsigned int> query;
   std::vector<ResultTuple> results;
   std::vector<std::string> q;
   std::string response = "";
   std::cout << "READY!" << std::endl;
   while (1) {
      query.clear();
      results.clear();
      HttpClientHandler client = server.run();
      q = client.parseRequest();
      std::cout << "Q.size = " << q.size() << std::endl;
      if (q.size() > 0) {
         std::string uri = q[0];
         std::cout << "URI = " << uri << std::endl;
         if (uri.find("/cache/") != std::string::npos) {
            std::string extra_hdr = "Content-Encoding: gzip" + CRLF;
			   response = mk_response_header("OK", 200, "text/html",
					extra_hdr);
            // Preparing the cached file
            uri.erase(0,7);
            std::cout << "Clean URI = " << uri << std::endl;
            unsigned int tmp;
            from_string<unsigned int>(tmp, uri, std::dec);
            uri = docDir(tmp);
            FILE* fh;
            if ((fh = fopen(uri.c_str(), "rb")) == NULL) {
               std::cout << "Can\'t open file: " << uri << std::endl;
            }
            int no = 0;
         	// Get it's fileno
	         if ( (no = fileno(fh)) == -1 ) {
               std::cout << "Can\'t get fileno.\n";
	         }
            struct stat stat_buf;
         	// Get it's filesize
	         if ( fstat(no, &stat_buf ) != 0 ) {
		         // stat failed
               std::cout << "Can\'t get filesize.\n";
         	}
   	      client.write(response);
			   sendfile( client.fd, no,
					NULL, stat_buf.st_size);
            fclose(fh);
            client.closeConnection();
            continue;
         } else {
            std::vector<std::string>::iterator it;
            // Get the term ids, if they exist.
            for (it = q.begin(); it != q.end(); ++it){
               std::cout << "Q term -> " << *it << std::endl;
               if (voc.find(*it) != voc.end()){
                  query.push_back(voc[*it]);
               }
            }
            std::cout << "Q size -> " << query.size() << std::endl;
            if (query.size() > 0) {
               results = vm.process(query);
               std::cout << "Results size -> " << results.size() << std::endl;
            }
         }
      }
      // Return answer to the client
      response = mk_response_header();
		response += mkResultPage(results);
   	client.write(response);
      client.closeConnection();
   }  
}

int main(int argc, char **argv) {
   if (strcmp(argv[1],"index") == 0){
      docId = 1244552;
      string file = getNextFile();
      // start the first run
      run = new vector<Ocurrences>;//(RUN_SIZE);
      while (file != ""){
      //for (int i = 0; i < 20000; ++i) {
         parse(file, docId-1);
         file = getNextFile();
      }
      // If there is something yet in the last run, sort it and store
      if (run->size() > 0) {
         storeRun();
      }
      delete run;
      // Merge the sorted runs
      //run_number = 191;
      vm.dumpTermFreq("dumpNi.txt");
      //vm.reloadTermFreq("dumpNi.txt");
      mergeRuns();
      vm.dumpNorms("dumpNorms.txt");
   } else if (strcmp(argv[1],"search") == 0){
      if (reloadIndex()) {
         searchInIndex();
      } else {
         cout << "No index to load...\n";
         return 1;
      }
   } else if (strcmp(argv[1],"vector") == 0){
      if (reloadIndex()) {
         // Starting the vector model class with data from our
         // stored files
         docId = 1244551;
         //docId = 20000;
         vm.startVecs(docId);
         vm.reloadTermFreq("dumpNi.txt");
         vm.reloadNorms("dumpNorms.txt");
         if (PAGERANK) {
            vm.loadPagerank();
         }
         search();
      } else {
         cout << "No index to load...\n";
         return 1;
      }
   } else if (argc < 1) {
      cout << "Must decide to \"index\" the collection " <<
         "or to \"search\" in an already indexed collection" << endl;
      return 1;
   }
}

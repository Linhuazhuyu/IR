#include <time.h>
#include <signal.h>
#include <vector>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <fstream>
#include <ext/hash_map>
#include <ext/hash_set>
#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto
#include "boost/filesystem/path.hpp"       // ditto
#include "collector.h"
#include "mycurl.h"
#include "parser.h"
#include "DataTypes.h"
#include "gatherer.h"
#include "beholder.h"
#include <queue>
#include "gzstream.h"
#include <algorithm>
#include "robots.h"

/*
 * Defining namespaces
 */
using namespace std;
namespace bfs=boost::filesystem;
namespace stdext=__gnu_cxx;

/*
 * Defining data structures and global vars
 */
// For the hash
struct eqstr{
  bool operator()(const std::string& s1, const std::string& s2) const{
    return s1 == s2;
  }
};

struct myhashfunc{
   stdext::hash<const char*> H;
   size_t operator()(const string& s1) const{
      return H(s1.c_str());
   }
};

typedef stdext::hash_map<string, queue<string>*, myhashfunc, eqstr> myHash;
typedef stdext::hash_set<PageCounter, myhashfunc2, eqpage> myHashSet;
typedef priority_queue<Domain, vector<Domain>, mycomparison> myPrQueue;

// Data structures used. Global to make life easier
myHash visitedDomains;
myPrQueue DomainQueue;
myHashSet visitedPages;
pthread_t gathererThreads[NUM_THREADS];
pthread_t beholderThread;

// Variables used to generate docID
pthread_mutex_t docIdMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t DomainQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t UrlQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t visitedPageMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t visitedDomainsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pagesInQueueMutex = PTHREAD_MUTEX_INITIALIZER;

unsigned long int docId = 0;
int currentDir = -1;
unsigned long int pagesInQueue = 0;

/*
 * Defining the functions
 */
// Converts a string to an unsigned long int (for restoring)
unsigned long int stoi(const string &s){
  istringstream myStream(s);  
  unsigned long int i;
  myStream>>i;
  return i;
}

// To save current state (for restoring)
void collector::saveState(){
   // Lock everything
   pthread_mutex_lock(&docIdMutex);
   pthread_mutex_lock(&DomainQueueMutex);
   pthread_mutex_lock(&UrlQueueMutex);
   pthread_mutex_lock(&visitedPageMutex);
   pthread_mutex_lock(&visitedDomainsMutex);
   // Iterate through the hash to get every page in the queues
   myHash::iterator it;
   //ofstream fout("resume.txt");
   ogzstream fout("resume.gz");
   for (it = visitedDomains.begin();
         it != visitedDomains.end(); it++){
      queue<string>* q = (queue<string>*)it->second;
      int qSize = q->size();
      string tmp;
      for (int i = 0; i < qSize; i++){
         tmp = q->front();
         q->pop();
         q->push(tmp);
         fout << tmp << endl;
      }
   }
   fout.close();
   //ofstream fout2("resume-seen.txt");
   ogzstream fout2("resume-seen.gz");
   fout2 << docId << endl; 
   myHashSet::const_iterator itSet;
   for (itSet = visitedPages.begin(); 
         itSet != visitedPages.end(); itSet++){
      fout2 << itSet->url << " " << itSet->counter << endl;
   } 
   fout2.close();
   pthread_mutex_unlock(&visitedDomainsMutex);
   pthread_mutex_unlock(&visitedPageMutex);
   pthread_mutex_unlock(&UrlQueueMutex);
   pthread_mutex_unlock(&DomainQueueMutex);
   pthread_mutex_unlock(&docIdMutex);
}
   
// Clean part of the visitedPages hahset 
// to keep the memory from exploding.
// Has no MUTEX because it was already taken
// when this function is called.
void visitedPagesGC(){
   if (visitedPages.size() > MAX_SET_SIZE){
      vector<PageCounter> tmp(visitedPages.begin(), visitedPages.end());
      sort(tmp.begin(), tmp.end());
      vector<PageCounter>::iterator it2;
      int i = 0;
      for (it2 = tmp.begin(); it2 != tmp.end(); it2++){
         if (i < MAX_SET_SIZE/4){
            tmp.erase(it2);
            i++; it2--;
         } else {
            break;
         }
      }
      visitedPages.clear();
      visitedPages.insert(tmp.begin(), tmp.end());
   }
}

// For Signal handling
void OnBreak(int nSig){
   // First save state
   cout << "Morrendo!\n";
   //collector::saveState();
   // Now kill the threads
   cout << "I'll have to cancel this sketch, 'cause it's too silly.\n";
   cout << "Killing threads:\n";
	for (int i = 0; i< NUM_THREADS; i++){
      pthread_kill(gathererThreads[i], SIGSTOP);
      cout << "Cancel signal sent to thread " << i << endl;
   }
   pthread_kill(beholderThread, SIGSTOP);
   cout << "Cancel signal sent to beholder thread " << endl;
   cout << "That's all, folks!\n";
}

bool isNew(string url, unsigned int count = 1){
   // Calls the garbage collector to keep the hash_set from exploding
   visitedPagesGC();
   // Verify if the page can be added to the queue
   PageCounter pc(url, count);
   myHashSet::iterator it = visitedPages.find(pc);
   if (it == visitedPages.end()){
      visitedPages.insert(pc);
      return true;
   } else {
      pc = *it;
      pc.addPageCount();
      visitedPages.erase(it);
      visitedPages.insert(pc);
      return false;
   }
}

// Print the domain priority queue
void printDomQueue(){
   while (!DomainQueue.empty()){
      Domain d = DomainQueue.top();
      DomainQueue.pop();
      cout << "Domain: " << d.domainName << "\n\tPages:\n";
      while (!d.pageQueue->empty()){
         string newUrl = d.pageQueue->front();
         d.pageQueue->pop();
         cout << "\t\t" << newUrl << endl;
      }
   }
}

// Print the pages pointed by the domain hash
void printDomHash(){
   myHash::iterator it;
   /*
         for (it=returnedLinks.begin(); it!=returnedLinks.end(); it++) {
            if (*it != ""){
               cout << "Adding page to queue: " << *it << endl;
               collector::insertUrl(*it);
            }
         }
   */
   for (it = visitedDomains.begin(); it != visitedDomains.end(); it++){
      cout << "Domain : " << it->first << "\n\tPages:\n";
   }
}

// Returns the docID for a page to use as filename 
string collector::getDocID(){
   div_t dir;
   string name = "";
   pthread_mutex_lock(&docIdMutex);
      dir = div(docId,DIR_SIZE);
      if (dir.quot > currentDir) {
         string tmp("collected/"+itos(dir.quot));
         bfs::create_directory(bfs::path(tmp));
      }
      name = "collected/" + itos(dir.quot) + "/" + itos(docId) + ".gz";
      docId++;
   pthread_mutex_unlock(&docIdMutex);
   return name;
}

// Returns the current docId. Only for statistics gathering 
unsigned long int collector::viewDocID(){
   unsigned long int currDocId;
   pthread_mutex_lock(&docIdMutex);
      currDocId = docId;
   pthread_mutex_unlock(&docIdMutex);
   return currDocId;
}


// Insert a new url in the queue
void collector::insertUrl(string url){
   bool isNewPage;
   pthread_mutex_lock(&pagesInQueueMutex);
   if (pagesInQueue < MAX_URLS_IN_QUEUE){
      pagesInQueue++;
      pthread_mutex_unlock(&pagesInQueueMutex);
   } else {
      pthread_mutex_unlock(&pagesInQueueMutex);
      return;
   }
   pthread_mutex_lock(&DomainQueueMutex);
   pthread_mutex_lock(&visitedPageMutex);
      if (url != ""){
         isNewPage = isNew(url);
      } else {
         // Just to get out of the loop. Must treat the
         // empty page later
         isNewPage = true;
      }
   pthread_mutex_unlock(&visitedPageMutex);
   if (url != "" && isNewPage){
      string domName = parser::domainName(url);
      // If the domain is new
      if (visitedDomains[domName] == NULL){
         // Create new queue for the pages in the new domain
         queue<string> *q = new queue<string>;
         // Create the new domain
         // Adds the TIME_BETWEEN_REQS because of the robots request
         time_t ts =time(NULL);
         Domain d(domName, ts, q);
         // Create a reference in the hash map to the queue
         pthread_mutex_lock(&visitedDomainsMutex);
            visitedDomains[domName] = q;
         pthread_mutex_unlock(&visitedDomainsMutex);
         // Put the new page in the right queue
         pthread_mutex_lock(&UrlQueueMutex);
            q->push(url);
         pthread_mutex_unlock(&UrlQueueMutex);
         // will exit the domain queue critical session while getting
         // the the robots.txt file. Will go back after that.
         // Probably no problem because the reference to the 
         // domain already exists
         
         pthread_mutex_unlock(&DomainQueueMutex);
         // Check if the domain has a "robots.txt"
         vector<string> rbts = getRobots(domName);
         if (!rbts.empty()){
            cout << "Domain " << domName << " has robots\n";
            d.addRobots(rbts);
         } else {
            //cout << domName << " has not...\n";
         }
         pthread_mutex_lock(&DomainQueueMutex);
         
         // Put the new domain in the priority queue
         DomainQueue.push(d);
      } else {
         pthread_mutex_lock(&visitedDomainsMutex);
            queue<string> *p = visitedDomains[domName];
            pthread_mutex_lock(&UrlQueueMutex);
               p->push(url);
            pthread_mutex_unlock(&UrlQueueMutex);
         pthread_mutex_unlock(&visitedDomainsMutex);
      }
   }
   pthread_mutex_unlock(&DomainQueueMutex);
}

/* 
 * Get the oldest domain in the Priority Queue (which probably
 * is available to download from), pick the first URL from it,
 * alter the timestamp accordingly (will receive the moment when 
 * it is ready to download again) and put the domain back in the PQ
 */ 
string collector::getUrl(){
pthread_mutex_lock(&DomainQueueMutex);
   if (!DomainQueue.empty()){
      // Get the first domain
      Domain d;
      //pthread_mutex_lock(&DomainQueueMutex);
         if (!DomainQueue.empty()){
            d = DomainQueue.top();
            DomainQueue.pop();
         } else {
            //cout << "Domain Queue empty!\n";
            pthread_mutex_unlock(&DomainQueueMutex);
            return "";
         }
      pthread_mutex_unlock(&DomainQueueMutex);
      // If The queue is not empty, we wilt try to get a url to download
      if (!d.pageQueue->empty()){
         // Verify the timestamp. Sleep 'till the time is right
         time_t now = time(NULL);
         if (d.timestamp > now){
            //cout << "Going to sleep for " << d.timestamp - now 
            //   << " secs. Good night.\n"; 
            sleep(d.timestamp - now);
            //cout << "I'm awake and good to go, sir!\n";
         }
         // Get the first url
         //bool isNewPage = false;
         string newUrl;
         bool okPage = false;
         bool pqEmpty = false;
         while (!pqEmpty && !okPage){
            pthread_mutex_lock(&UrlQueueMutex);
               if (!d.pageQueue->empty()){
               newUrl = d.pageQueue->front();
               d.pageQueue->pop();
               } else {
                  pqEmpty = true;
                  continue;
               }
            pthread_mutex_unlock(&UrlQueueMutex);
            pthread_mutex_lock(&pagesInQueueMutex);
               pagesInQueue--;
            pthread_mutex_unlock(&pagesInQueueMutex);
            okPage= true;
            
            string dir = newUrl;
            dir.erase(0,parser::domainName(dir).size());
            if (d.robotsPermit(dir)){
               okPage = true;
               //cout << "Ok pelo robots -> " << newUrl << endl;
            } else {
               //cout << "Não posso dar entrada nessa merda! -> "
               //   << newUrl << endl;
            }
            
         }
         if (!okPage) newUrl = "";
         pthread_mutex_unlock(&UrlQueueMutex);
         // Change the domain timestamp
         d.timestamp = time(NULL)+WAIT_BETWEEN_REQS;
         // Return it to the PQ
         pthread_mutex_lock(&DomainQueueMutex);
         DomainQueue.push(d);
         pthread_mutex_unlock(&DomainQueueMutex);
         return newUrl;
      } else {
         d.timestamp = time(NULL)+2*WAIT_BETWEEN_REQS;
         DomainQueue.push(d);
         pthread_mutex_unlock(&DomainQueueMutex);
         return "";
         
      }
   } else {
      //cout << "Domain queue empty?? but... why?!?" << endl;
      pthread_mutex_unlock(&DomainQueueMutex);
      return "";
   }
pthread_mutex_unlock(&DomainQueueMutex);
}

void fillQueue(const char* infile){
   string url;
   igzstream myfile(infile);
   getline (myfile,url);
      while (!myfile.eof()){
         // will call function to insert url
         //cout << "Going to insert -> " << url << endl;
         collector::insertUrl(url);
         getline (myfile,url);
      }
      myfile.close();
   cout << "Finished reloading the seeds/resume queue\n";
}

void fillKnown(const char* infile){
   string url, tmp;
   unsigned int count, pos;
   igzstream myfile(infile);
      getline (myfile,url);
      docId = stoi(url);
      getline (myfile,url);
      while (!myfile.eof()){
         // will call function to insert url
         //cout << "Going to insert -> " << url << endl;
         pos = url.rfind(" ");
         if (pos != string::npos){
            tmp.assign(&url[pos+1]);
            count = stoi(tmp);
            url.erase(pos);
            isNew(url, count);
         } else {
            isNew(url);
         }
         getline (myfile,url);
      }
   myfile.close();
   cout << "Finished reloading the known URLs\n";
   //} else cout << "Unable to open file!!";
}

/*
 * Defining the main function
 */
int main(int argc, char **argv){
   // Preparing to handle signals
   signal(SIGTERM,OnBreak);
   signal(SIGINT,OnBreak);
   signal(SIGPIPE,OnBreak);
   signal(SIGSEGV,OnBreak);
   signal(SIGABRT,OnBreak);
   // Verify is it is beginning a new crawl or resuming an old one
   if (strcmp(argv[1],"start") == 0){
      cout << "Starting work...\n";
      fillQueue("seeds.gz");
      //fillQueue("bugs.txt");
   } else if (strcmp(argv[1],"resume") == 0){
      cout << "Resuming work...\n";
      fillQueue("resume.gz");
      fillKnown("resume-seen.gz");
   } else {
      cout << "Must decide to \"start\" or \"resume\" crawling." << endl;
      return 1;
   }
   // Starting the threads
   //gatherer::gatherPages();
   int error;
   for(int i=0; i< NUM_THREADS; i++) {
      error = pthread_create(&gathererThreads[i],
                NULL, // default attributes
                gatherer::gatherPages,
                NULL);
      if(error != 0){
         fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);
      }
      sleep(1);
   }
   error = pthread_create(&beholderThread,
                NULL, // default attributes
                beholder::gatherStatistics,
                NULL);
   if(error != 0){
      fprintf(stderr, "Couldn't run beholder thread errno %d\n", error);
   }

   // Wait for threads to end
   for(int i=0; i< NUM_THREADS; i++) {
      error = pthread_join(gathererThreads[i], NULL);
      fprintf(stderr, "Thread %d terminated\n", i);
   }
   error = pthread_join(beholderThread, NULL);
   fprintf(stderr, "Beholder thread terminated\n");
   return 0;
}

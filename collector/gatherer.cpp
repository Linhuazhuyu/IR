/*
 * Thread class. Will be responsible for getting the pages (with libcurl).
 * Maybe it will also store the pages. Probably will call the parser class,
 * but it can change if we use 2 kinds of threads (not likely).
 */
#include <iostream>
#include <fstream>
#include <set>
#include "mycurl.h"
#include "collector.h"
#include "gatherer.h"
#include "parser.h"
#include "DataTypes.h"

using namespace std;

void *gatherer::gatherPages(void* noUse){
   bool stop = false;
   int num = 0;
   set<string> returnedLinks;
   while (!stop){
      string url = collector::getUrl();
      if (url == ""){
      } else {
         //cout << "Will get page: " << url << endl;
         // Initializing stuff
         struct PageStruct chunk;
         mycurl::curlInit();
         // Getting the page
         chunk = mycurl::getPage(url);
         // Extract links and save to disk
         //cout << chunk.memory << "\n\n";
         if (chunk.size > 0){
            returnedLinks = parser::getLinks(chunk,url);
            // Clean memory
            mycurl::clearChunk(chunk);
            // Add the obtained links to their respective queues
            set<string>::iterator it;
            for (it=returnedLinks.begin(); it!=returnedLinks.end(); it++) {
               if (*it != ""){
                  //cout << "Adding page to queue: " << *it << endl;
                  collector::insertUrl(*it);
               }
            }
         }
      }
   }
   return NULL;
}

/*
 * Will give support to read and evaluate robots
 */
#include "mycurl.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include "DataTypes.h"
#include "robots.h"

using namespace std;

vector<string> getRobots(string url){
   vector<string> robotsDisallow;
   url += "/robots.txt";
   PageStruct chunk;
   string robots, line;
   mycurl::curlInit();
   chunk = mycurl::getPage(url);
   if (!(chunk.size > 0)){
      mycurl::clearChunk(chunk);
      return robotsDisallow;
   }
   robots.assign(chunk.memory);
   mycurl::clearChunk(chunk);
   istringstream ss(robots);
   bool isRobots = false;
   do {
      getline(ss,line);
      if (line[0] == '#'){
         continue;
      } else {
         transform(line.begin(), line.end(), line.begin(), 
           (int(*)(int)) tolower);
         if (line.compare(0,10,"user-agent") == 0){
            isRobots = true;
         }
         break;
         
      }
   } while(!ss.eof());
   if (isRobots) {
      bool storeInfo = false;
      do {
         transform(line.begin(), line.end(), line.begin(), 
           (int(*)(int)) tolower);
         if (line.compare(0,10,"user-agent") == 0) {
            int pos;
            pos = line.find("*",10);
            if (pos != line.npos) {
               // "That's us on the tape!" everything till
               // the endo of file or new user-agent must be stored
               storeInfo = true;
            } else { 
               // Shall ignore everything till a new user-agent
               storeInfo = false; 
            }
         } else if (storeInfo){
            // get rid of those pesky white spaces
            while (line[0] == ' '){
               line.erase(0,1);
            }
            if (line[0] == '\n') {
               // nothing to do besides waiting for a new getline...
            } else if (tolower(line[0]) == 'd'){
               // will only store "disallow"
               string tmp;
               tmp.assign(&line[0],9);
               transform(tmp.begin(), tmp.end(), tmp.begin(), 
                  (int(*)(int)) tolower);
               if (tmp == "disallow:") {
                  line.erase(0,9);
                  robotsDisallow.push_back(trim(line));
               } else { cout << "Can trust no one these days...\n";}
            }
         }
         getline(ss,line);
      } while (!ss.eof());
   } else {
      //cout << "No go. Bugger off!\n";
   }
   return robotsDisallow;
}

#include <string>
#include <time.h>
#include <exception>
#include <iostream>
#include <sstream>
#include <ext/hash_set>
#include "DataTypes.h"
#include "parser.h"

namespace stdext=__gnu_cxx;

//class MyException: public std::exception{};


// Converts int to string
std::string itos(int i){	
   std::stringstream s;
   s << i;
   return s.str();
}

Domain::Domain(){};

Domain::Domain(std::string domName, time_t tstamp, 
      std::queue<std::string>* pQueue){
   domainName = domName;
   timestamp = tstamp;
   pageQueue = pQueue;
   hasRobots = false;
}

void Domain::addRobots(std::vector<std::string> vec){
   if (!vec.empty() > 0){
      hasRobots = true;
      std::vector<std::string>::iterator it;
      for (it = vec.begin(); it != vec.end(); it++){
         robotsDisallow.push_back(*it);
      }
   }
}

bool Domain::robotsPermit(std::string link){
   if (hasRobots){
      std::vector<std::string>::iterator it;
      for (it = robotsDisallow.begin(); 
            it != robotsDisallow.end(); it++){
         if (link.size() >= it->size()){
            if (link.compare(0,it->size(),*it) == 0){
               return false;
            }
         }
      }
      // If has verified all the robots and it's still here,
      // then it can be downloaded
      return true;
   } else {return true;}
}

PageCounter::PageCounter(){};

PageCounter::PageCounter(std::string newUrl, unsigned int count){
   url = newUrl;
   counter = count;
}

void PageCounter::addPageCount(){
   counter++;
}

// Necesary to allow a heap of domain
bool mycomparison::operator() (const Domain& d1, const Domain& d2) const {
   return (d1.timestamp > d2.timestamp);
}

// Necessary for the hash_set "garbage collector"
bool operator<(const PageCounter& pc1, const PageCounter& pc2){
   return (pc1.counter < pc2.counter);
}

std::string trim (std::string s){
   if (s != ""){
      while (s[0] == ' ' && s.size() > 0){
         s.erase(0,1);
      }
      if (s.size() > 0){
         while (s[s.size()-1] == ' ' && s.size() > 0){
         s.erase(s.size()-1,1);
         }
      }
      return s;
   } else return "";
}

/*
 * Presents the classes that will store the ocurrences of
 * index terms while parsing te filse (Ocurrences) and when
 * merging the sorted runs (RunOcurrence)
 */


#include "ocurrences.h"
#include "utils.h"
#include <iostream>

// Necessary to sort Ocurrences
bool operator<(const Ocurrences& o1, const Ocurrences& o2){
   if (o1.term < o2.term) {
      return true;
   } else if (o1.term == o2.term) {
      if (o1.document < o2.document) {
         return true;
      } else {
         return false;
      }
   } else {
      return false;
   }
}

Ocurrences::~Ocurrences(){}

Ocurrences::Ocurrences(){}

Ocurrences::Ocurrences(unsigned int t, unsigned int d, unsigned int f){
      term = t; document = d; frequency = f;
}

bool Ocurrences::operator==(const Ocurrences& o){
   return ((this->term == o.term) && (this->document == o.document));
}

// Necessary to the heap of RunOcurrences (?)
bool operator<(const RunOcurrence& ro1, const RunOcurrence& ro2){
   if (ro1.term < ro2.term) {
      return true;
   } else if (ro1.term == ro2.term) {
      if (ro1.document < ro2.document) {
         return true;
      } else {
         return false;
      }
   } else {
      return false;
   }
}

RunOcurrence::~RunOcurrence(){}

RunOcurrence::RunOcurrence(){}

RunOcurrence::RunOcurrence(unsigned int t, unsigned int d, 
      unsigned int f, unsigned int r){
      term = t; document = d; frequency = f; run = r;
}

RunOcurrence::RunOcurrence(Ocurrences o, unsigned int r){
      term = o.term; document = o.document; 
      frequency = o.frequency; run = r;
}

void RunOcurrence::splitOcurrence(std::string oc, unsigned int& term, 
      unsigned int& doc, unsigned int& freq) {
   std::string tmp;
   int pos = oc.find(";");
   tmp.assign(oc.c_str(),pos);
   oc.erase(0,pos+1);
   from_string<unsigned int>(term, tmp, std::dec); 
   pos = oc.find(";");
   tmp.assign(oc.c_str(),pos);
   oc.erase(0,pos+1);
   from_string<unsigned int>(doc, tmp, std::dec);
   from_string<unsigned int>(freq, oc, std::dec);
}

RunOcurrence::RunOcurrence(std::string s, unsigned int r){
   splitOcurrence(s, term, document, frequency);
   run = r;
}

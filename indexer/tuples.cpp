#include "tuples.h"

ResultTuple::ResultTuple(unsigned int docid, double sim){
   doc_id = docid;
   similarity = sim;
}

ResultTuple::ResultTuple(){
}

ResultTuple::~ResultTuple(){
}

bool operator<(const ResultTuple& rt1, const ResultTuple& rt2){
   return (rt2.similarity < rt1.similarity);
}

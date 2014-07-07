#ifndef TUPLES_H
#define TUPLES_H
struct tuple{
   unsigned int docId;
   unsigned int freq;
};

// Result tuple
class ResultTuple{
   public:
      ResultTuple(unsigned int docid, double sim);
      ResultTuple();
      ~ResultTuple();
      unsigned int doc_id;
      double similarity;
};

bool operator<(const ResultTuple& rt1, const ResultTuple& rt2);
#endif

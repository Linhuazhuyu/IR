#include <vector>
#include <ext/hash_map>
#include <string>
#include "tuples.h"

#define PAGERANK 1
#define VECTOR_WEIGHT 0.3

typedef __gnu_cxx::hash_map<unsigned int, float> Uint32FloatHmap;
typedef __gnu_cxx::hash_map<unsigned int, unsigned int> uint_hmap;
typedef __gnu_cxx::hash_map<unsigned int, double> acc_hmap;

class VectorModel{
   public:
      // adds to norm accumulator
      void normAcc(unsigned int docid, unsigned int termid, 
            unsigned int freqij);
      // calculates the norms. uses the accumulated values
      void calculateNorms();
      VectorModel();
      ~VectorModel();
      // Adds to a term frequency in the collection (ri)
      void addTermFrequency(unsigned int term_id);
      void startVecs(unsigned int col_size);
      void dumpTermFreq(std::string filename);
      void dumpNorms(std::string filename);
      void reloadNorms(std::string filename);
      void reloadTermFreq(std::string filename);
      std::vector<ResultTuple> process(std::vector<unsigned int>& query);
      void loadPagerank();
   private:
      // document norm accumulator
      std::vector<double> doc_norm;
      // Maximum tf in a document
      std::vector<unsigned int> max_freq;
      // Number of documents with a given term (ni)
      uint_hmap docs_with_term;
      unsigned int collection_size;
      void setMaxTF(unsigned int docid, unsigned int freq);
      void openTermFile(std::ifstream& termList, unsigned int termId, 
         unsigned long int& length);
      double weigth(unsigned int term_id, unsigned int doc_id,
            unsigned int freqij);
      Uint32FloatHmap prank_values;
};

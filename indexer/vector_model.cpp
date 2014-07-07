#include <math.h>
#include "vector_model.h"
#include <fstream>
#include <iostream>
#include "utils.h"
#include "gzstream.h"
#include <ext/hash_map>

void VectorModel::normAcc(unsigned int docid, unsigned int termid, 
      unsigned int freqij){
   if (freqij > 0) {   
      doc_norm[docid] += pow(
            freqij*(log(collection_size/(double)docs_with_term[termid])), 
            2.0);
      setMaxTF(docid,freqij);
   }
}

void VectorModel::setMaxTF(unsigned int docid, unsigned int freq){
   if (freq > max_freq[docid]) {
      max_freq[docid] = freq;
   }
}

void VectorModel::calculateNorms() {
   for (int i = 0; i < collection_size; ++i) {
      doc_norm[i] = (double)sqrt(doc_norm[i])/max_freq[i];
   }
}

VectorModel::~VectorModel(){
}

void VectorModel::addTermFrequency(unsigned int term_id){
   if (docs_with_term.find(term_id) == docs_with_term.end()) {
      docs_with_term[term_id] = 1;
   } else {
      ++docs_with_term[term_id];
   }
}

VectorModel::VectorModel() {
}

void VectorModel::startVecs(unsigned int col_size) {
   collection_size = col_size;
   for (int i = 0; i < collection_size; ++i) {
      doc_norm.push_back(0.0);
      max_freq.push_back(0);
   }
}

// Prints the term frequency in the collection (ni) into a file 
void VectorModel::dumpTermFreq(std::string filename){
   std::ofstream outfile(filename.c_str(), std::ios::binary);
   uint_hmap::iterator it;
   for (it = docs_with_term.begin(); it != docs_with_term.end(); ++it) {
      outfile.write((char*)&it->first,sizeof(unsigned int));
      outfile.write((char*)&it->second,sizeof(unsigned int));
      //outfile << it->first << ";" << it->second << std::endl;
   }
   outfile.close();
}

// Prints the norm values (and other stuff too) into a file 
void VectorModel::dumpNorms(std::string filename){
   std::ofstream outfile(filename.c_str(), std::ios::binary);
   for (int i = 0; i < collection_size; ++i) {
      outfile.write((char*)&i,sizeof(unsigned int));
      outfile.write((char*)&doc_norm[i],sizeof(double));
      outfile.write((char*)&max_freq[i],sizeof(unsigned int));
      //outfile << i << ";" << doc_norm[i] << ";" << max_freq[i] << std::endl;
   }
   outfile.close();
}

// Reloads the norm values (and other stuff too) from a file 
void VectorModel::reloadNorms(std::string filename){
   unsigned int aux;
   char damn[sizeof(double)];
   std::ifstream infile(filename.c_str(), std::ios::binary); 
   for (int i = 0; i < collection_size; ++i) {
      infile.read((char*)&aux,sizeof(unsigned int));
      infile.read((char*)&doc_norm[aux],sizeof(double));
      infile.read((char*)&max_freq[aux],sizeof(unsigned int));
   }
   infile.close();
}

// Reloads the term frequency in the collection (ni) from a file 
void VectorModel::reloadTermFreq(std::string filename){
   std::ifstream infile(filename.c_str(), std::ios::binary);
   // get file length
   infile.seekg (0, std::ios::end);
   unsigned int length = infile.tellg();
   infile.seekg (0, std::ios::beg);
   unsigned int currentPos = infile.tellg();
   unsigned int key, value;
   // now read the file
   while (currentPos < length){
      infile.read((char*)&key,sizeof(unsigned int));
      infile.read((char*)&value,sizeof(unsigned int));
      docs_with_term[key] = value;
      currentPos = infile.tellg();
   }
   infile.close();
}

// returns the answer for the query, ordered by similarity
std::vector<ResultTuple> VectorModel::process(std::vector<unsigned int>& query) {
   std::vector<unsigned int>::iterator it;
   float max_prank = 0.0, max_sim = 0.0;
   acc_hmap accumulator;
   // for each query term
   if (query.size() > 0) {
      std::ifstream handlers[query.size()];
      unsigned long int lengths[query.size()];
      tuple aux[query.size()];
      unsigned int current_doc[query.size()];
      for (int i = 0; i < query.size(); ++i){
         handlers[i].open(termDir(query[i]).c_str(), std::ios::binary);
         lengths[i] = fileLength(handlers[i]);
         current_doc[i] = 0;
      }
      if (query.size() > 1){
         bool same_doc;
         while (handlers[0].tellg() < lengths[0]) {
            handlers[0].read ((char*)&aux[0], sizeof(tuple));
            current_doc[0] += aux[0].docId;
            // find the docids of the other lists that are not lower than
            // the current doc
            for (int i = 1; i < query.size(); ++i) {
               do {
                  handlers[i].read ((char*)&aux[i], sizeof(tuple));
                  current_doc[i] += aux[i].docId;
               } while (current_doc[i] < current_doc[0] 
                     && handlers[i].tellg() < lengths[i]);
            }
            same_doc = true;
            // verify if it is the same doc
            for (int i = 1; i < query.size(); ++i) {
               same_doc = same_doc && (current_doc[i-1] == current_doc[i]);
            }
            if (same_doc) {
               if (accumulator.find(current_doc[0]) == accumulator.end()){
                  // new accumulator
                  accumulator[current_doc[0]] = 0.0;
               }
               // Calculate doc similarity
               for (int i = 0; i < query.size(); ++i) {
                  accumulator[current_doc[0]] += 
                     weigth(query[i],current_doc[i],aux[i].freq)
                     /(double)doc_norm[current_doc[i]];
               }
               // evaluate max pagerank for normalization purposes
               if (PAGERANK) {
                  if (max_prank < prank_values[current_doc[0]]) {
                     max_prank = prank_values[current_doc[0]];
                  }
               }
               // evaluate max similarity for normalization purposes
               if (max_sim < accumulator[current_doc[0]]) {
                  max_sim = accumulator[current_doc[0]];
               }
            }
         }
      } else {
         // just one term. Proceed as usual
         while (handlers[0].tellg() < lengths[0]) {
            handlers[0].read ((char*)&aux[0], sizeof(tuple));
            current_doc[0] += aux[0].docId;
            if (accumulator.find(current_doc[0]) == accumulator.end()){
               // new accumulator
               accumulator[current_doc[0]] = 0.0;
            }
            accumulator[current_doc[0]] += 
                     weigth(query[0],current_doc[0],aux[0].freq)
                     /(double)doc_norm[current_doc[0]];
         }
         // evaluate max pagerank for normalization purposes
         if (PAGERANK) {
            if (max_prank < prank_values[current_doc[0]]) {
               max_prank = prank_values[current_doc[0]];
            }
         }
         // evaluate max similarity for normalization purposes
         if (max_sim < accumulator[current_doc[0]]) {
            max_sim = accumulator[current_doc[0]];
         }
      }
      for (int i = 0; i < query.size(); ++i){
         handlers[i].close();
      }
   }
   acc_hmap::iterator rit;
   if (PAGERANK) {
      // normalize it with pagerank
      for (rit = accumulator.begin(); rit != accumulator.end(); ++rit) {
         rit->second = VECTOR_WEIGHT*(rit->second/(float)max_sim) 
            + (1-VECTOR_WEIGHT)*(prank_values[rit->first]/(float)max_prank);
      }
   } else {
      // just normalize the similarity values
      for (rit = accumulator.begin(); rit != accumulator.end(); ++rit) {
         rit->second = rit->second/(float)max_sim;
      }
   }
   // We have all the normalized similarities. Now sort it and return
   std::vector<ResultTuple> result;
   for (rit = accumulator.begin(); rit != accumulator.end(); ++rit) {
      result.push_back(ResultTuple(rit->first,rit->second));
            // /(double)doc_norm[rit->first]));
   }
   sort(result.begin(), result.end());
   return result;
}

double VectorModel::weigth(unsigned int term_id, unsigned int doc_id, 
      unsigned int freqij) {
   return ((freqij/(double)max_freq[doc_id])* //tf
      (log(collection_size/(double)docs_with_term[term_id]))); //idf
}

void VectorModel::loadPagerank(){
   igzstream prankFile;
   prankFile.open("pagerank-values.gz");
   std::string entry;
   getline(prankFile,entry);
   while (!prankFile.eof()){
      unsigned int pos = entry.find(" ");
      float value;
      std::string node;
      node.assign(entry.c_str(),pos);
      entry.erase(0,pos+1);
      from_string<unsigned int>(pos, node, std::dec);
      from_string<float>(value, entry, std::dec);
      prank_values[pos] = value;
      getline(prankFile,entry);
   }
   prankFile.close();
}

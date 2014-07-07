/* Will get the pages we acquired with crazy_collector
 * and extract all the tags from it. Must remember that 
 * (at least) the following tags must have the content 
 * between their open-close tags ignored: textarea, 
 * script, comments and style.
 */
#include "zfilebuf.h"
#include "gzstream.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <ext/hash_map>
#include <tr1/functional>
#include "utils.h"
#include "parser.h"
#include "graph.h"

// The last docId we have
#define LAST_DOC 1244550
#define DIR_SIZE 10000

using namespace std;

typedef __gnu_cxx::hash_map < std::string, unsigned int,
                        std::tr1::hash<std::string> > StrUint32Map;

typedef __gnu_cxx::hash_map<unsigned int, float> UintDoubleHMap;

const float base_prank = 1.0;
const float prank_error = 5.0;
const float prank_damp = 0.85;


// Returns the location of the next file to process
std::string getNextFile(unsigned long int &docId){
   if (docId > LAST_DOC) {
      return "";
   } else {
      div_t dir;
      dir = div(docId,DIR_SIZE);
      string tmp = "../colector/collected-bk/" + itos(dir.quot) 
         + "/" + itos(docId) + ".gz";
      //string tmp = "stripped/" + itos(dir.quot) 
      //   + "/" + itos(docId) + ".gz";
      ++docId;
      return tmp;
   }
}

std::string getUrl(memory_buffer& dec) {
   string aux = "";
   unsigned int i = 0;
   while (i < dec.second) {
      if (dec.first[i] == 'M') {
         aux.assign(&dec.first[i], 8);
         if (aux == "MYURL = ") {
            i += 8;
            aux = "";
            while (dec.first[i] != ' '){
               aux += dec.first[i];
               ++i;
            }
            break;
         }
      }
      ++i;
   }
   return aux;
}

void pageRankCalculus(Graph& g){
   UintDoubleHMap prank_vals, prank_vals_acc;
   // Initialize prank_vals
   hmap::iterator it;
   set<unsigned int>::iterator sit;
   for (it = g.graph_map.begin(); it != g.graph_map.end(); ++it){
      prank_vals[it->first] = base_prank;
      prank_vals_acc[it->first] = 0.0;
   }
   // repeat calculus until it converges
   float error = 10.0, prank_aux, avg_error = 0.0;
   unsigned int num_neighbors;
   unsigned int round = 0;
   bool converged = false;
   unsigned int g_size = g.graph_map.size();
   while (not converged) {
      converged = true;
      avg_error = 0.0;
      // Accumulating the votes      
      for (it = g.graph_map.begin(); it != g.graph_map.end(); ++it){
         num_neighbors = it->second->size();
         for (sit = it->second->begin(); sit != it->second->end(); ++sit){
            prank_vals_acc[*sit] += prank_vals[it->first]/(float)num_neighbors;
         }
      }
      // applying damping factor and zeroing accumulators
      for (it = g.graph_map.begin(); it != g.graph_map.end(); ++it){
          prank_aux = (1-prank_damp)+(prank_damp*prank_vals_acc[it->first]);
          if (prank_aux > prank_vals[it->first]) {
             error = prank_aux - prank_vals[it->first];
          } else {
             error = prank_vals[it->first] - prank_aux;
          }
          avg_error += error;
          //if (error > prank_error) converged = false;
          //std::cout << "Error -> " << error << std::endl;
          prank_vals[it->first] = prank_aux;
          prank_vals_acc[it->first] = 0.0;
      }
      if (avg_error > prank_error) converged = false;
      ++round;
      std::cout << "# Rodadas: " << round << ", Error = " 
         << avg_error/(float)g_size << std::endl;
   }
   //std::cout << "The removed nodes are: ";
   for (sit = g.sinks.begin(); sit != g.sinks.end(); ++sit) {
      //std::cout << *sit << " ";
      prank_vals[*sit] = 1-prank_damp;
   }
   //std::cout <<std::endl;
   // saving page rank to a file
   ogzstream prank_out;
   prank_out.open("pagerank-values.gz");
   UintDoubleHMap::iterator prit;
   for (prit = prank_vals.begin(); prit != prank_vals.end(); ++prit){
      prank_out << prit->first << " " << prit->second << std::endl;
   }
   prank_out.close();
}

// Parse a file to remove any tags. Will also treat &; chars
std::string getUrls(string file){
   // decompressing the file
   memory_buffer dec(0,0);
	try{
		dec = decompres(file);
	} catch(...) {
	   // pass
      std::cout << "Error opening file!\n";
   }
   //cout << dec.first << endl;
   // create destiny directory and output file
   unsigned int i = 0;
   std::string aux;
   aux = getUrl(dec);
	delete[] dec.first;
   return aux;
}

void reloadPages(StrUint32Map& pages) {
   igzstream infile;
   infile.open("indexed-urls.gz");
   std::string entry;
   getline(infile,entry);
   int docid = 0;
   while(not infile.eof()) {
      pages[entry] = docid;
      ++docid;
      getline(infile,entry);
   }
}

int main(int argc, char **argv){
   if (strcmp(argv[1],"first_run") == 0){
      unsigned long int docId = 0;
      //docId = 675705;
      string file = getNextFile(docId);
      ogzstream outfile;
      outfile.open("indexed-urls.gz");
      while (file != ""){
         outfile << getUrls(file) << std::endl;
         file = getNextFile(docId);
      }
      outfile.close();
   } else if (strcmp(argv[1],"parse") == 0){
      unsigned long int docId = 0;
      //docId = 675705;
      StrUint32Map pages;
      reloadPages(pages);
      string file = getNextFile(docId);
      ogzstream outfile;
      outfile.open("indexed-links.gz");
      memory_buffer dec(0,0);
      set<string> links;
      set<string>::iterator it;
      while (file != ""){
         links.clear();
	      try{
		      dec = decompres(file);
      	} catch(...) {
	         // pass
            std::cout << "Error opening file!\n";
         }
         string base = getUrl(dec);
         //base = parser::domainName(base);
         links = parser::getLinks(dec,base);
         for (it = links.begin(); it != links.end(); ++it){
            //std::cout << *it << std::endl;
            if (pages.find(*it) != pages.end()){
               outfile << pages[base] << " " << pages[*it] << std::endl;
            }
         }
         delete[] dec.first;
         file = getNextFile(docId);
      }
      outfile.close();
   } else if (strcmp(argv[1],"calculate") == 0){
      Graph g("indexed-links.gz");
      //g.removeSinks();
      pageRankCalculus(g);
   }
}

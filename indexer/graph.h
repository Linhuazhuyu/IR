#include <ext/hash_map>
#include <vector>
#include <set>

typedef __gnu_cxx::hash_map<unsigned int, std::set<unsigned int>*> hmap; 

// Class definition. Will probably grow.
class Graph{
   private:
      unsigned int num_nodes;
      unsigned int num_edges;
      void addEdge(unsigned int node1, unsigned int node2);
   public:
      void fill(std::string filename);
      Graph(std::string filename);
      Graph();
      ~Graph();
      void print();
      hmap graph_map;
      void removeSinks();
      std::set<unsigned int> sinks;
};

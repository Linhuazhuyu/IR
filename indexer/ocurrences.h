#include <string>

class Ocurrences{
   public:
      bool operator==(const Ocurrences& o);
      unsigned int term;
      unsigned int document;
      unsigned int frequency;
      Ocurrences(unsigned int t, unsigned int d, unsigned int f);
      Ocurrences();
      ~Ocurrences();   
};

bool operator<(const Ocurrences& o1, const Ocurrences& o2);

class RunOcurrence : public Ocurrences{
   public:
      RunOcurrence(unsigned int t,unsigned int d,
            unsigned int f,unsigned int r);
      RunOcurrence(Ocurrences o, unsigned int r);
      RunOcurrence(std::string s, unsigned int r);
      RunOcurrence();
      ~RunOcurrence();
      unsigned int run;
   private:
      void splitOcurrence(std::string oc, unsigned int& term, 
            unsigned int& doc, unsigned int& freq);
};

bool operator<(const RunOcurrence& ro1, const RunOcurrence& ro2);

/*
 * Elias Gamma encoding and decoding. Since it is has a lot
 * of bit manipulation, the code is just fugly. Deal with it.
 */

#include "egamma.h"
#include "utils.h"
#include <cmath>

// Encodes a value using Elias Gamma
std::string eliasGammaCode(unsigned int num) {
   unsigned int log = (unsigned int)floor(log2(num));
   unsigned int rval = num - log;
   std::vector<bool> res;
   std::string rstr = "";
   // log num will be represented as unary code
   for (int i = 0; i < log; i++) {
      res.push_back(true);
   }
   res.push_back(false);
   // the remaining will be coded as plain binary and appended
   for (int i=0; i < log; i++) {
      if (num & 1 << i)
         rstr = '1' + rstr;
      else
         rstr = '0' + rstr;
   }
   for (int i=0; i < log; i++) {
      if (rstr[i] == '1') {
         res.push_back(true);
      } else {
         res.push_back(false);
      }
   }
   // the following parts are here just because c expects always
   // bytes no work with. We will shove zeros to fill a byte 
   // (if necessary) and create a unsigned chars with so we 
   // can store them
   int rest = 8 -(res.size() % 8);
   for (int i = 0; i < rest; ++i) {
      res.push_back(false);
   }
   unsigned char tmp = 0;
   int count = 0;
   std::string out = "";
   for (int i = 0; i < res.size(); ++i) {
      tmp<<=1;
      if(res[i]){tmp|=1;}
      if (count == 7) {
         out +=tmp;
         tmp = 0;
         count = 0;
      } else { 
         ++count;
      }
   }
   return out;
}

// Decode a Elias Gamma value
unsigned int eliasGammaDecode(std::string strIn) {
   // Since we are treating bytes here (filled with zeros in the
   // end if necessary), we must first put everything in a better
   // representation to work with
   unsigned char tmp;
   std::vector<bool> in(strIn.length()*8);
   for (int i = 0; i < strIn.length(); ++i) {
      tmp = strIn[i];
      for (int j = 7; j >= 0; --j) {
         if ((int)(00000001 & tmp) == 1) {
            in[j+(8*i)] = true;
         } else {
            in[j+(8*i)] = false;
         }
         tmp >>= 1;      
      }
   }
   // Start decoding the little bastard
   unsigned int p = 0;
   unsigned int res = 0;
   while (in[0]) {
      ++p;
      in.erase(in.begin());
   }
   in.erase(in.begin());
   res += myPow(2,p);
   // only the next p bits form a value
   while (in.size() > p) in.erase(in.end());
   // the rest is a binary number
   unsigned int count = 0;
   std::vector<bool>::reverse_iterator rit;
   for (rit = in.rbegin(); rit != in.rend(); ++rit) {
      if (*rit) {
         res+= myPow(2,count);
      }
      ++count;
   }
   return res;
}

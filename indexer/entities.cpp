#include "entities.h"
#include <iconv.h>
#include "utils.h"
#include <iostream>

Entities::Entities() {
   Entities::initEntityHash();
}

Entities::~Entities() {
}

// Convert Unicode codepoint to it's UTF-8 representation.
std::string fromUnicodeToUTF(unsigned int uv) {
	const int UTF_MAX  = 6;	/* They can be bigger, but we just don't care */

	char output[UTF_MAX +1] = {0,0,0,0,0,0,0}; /* Strings will
                         						     * be null-termiinated
						                             * no matter what
						                             */

	char* d = output;

	if (uv < 0x80) {
		*d++ = uv;
		return std::string(output);
	}
	if (uv < 0x800) {
		*d++ = (( uv >>  6)         | 0xc0);
		*d++ = (( uv        & 0x3f) | 0x80);
		return std::string(output);
	}
	if (uv < 0x10000) {
		*d++ = (( uv >> 12)         | 0xe0);
		*d++ = (((uv >>  6) & 0x3f) | 0x80);
		*d++ = (( uv        & 0x3f) | 0x80);
		return std::string(output);
	}
	if (uv < 0x200000) {
		*d++ = (( uv >> 18)         | 0xf0);
		*d++ = (((uv >> 12) & 0x3f) | 0x80);
		*d++ = (((uv >>  6) & 0x3f) | 0x80);
		*d++ = (( uv        & 0x3f) | 0x80);
		return std::string(output);
	}
	if (uv < 0x4000000) {
		*d++ = (( uv >> 24)         | 0xf8);
		*d++ = (((uv >> 18) & 0x3f) | 0x80);
		*d++ = (((uv >> 12) & 0x3f) | 0x80);
		*d++ = (((uv >>  6) & 0x3f) | 0x80);
		*d++ = (( uv        & 0x3f) | 0x80);
		return std::string(output);
	}
	if (uv < 0x80000000) {
		*d++ = (( uv >> 30)         | 0xfc);
		*d++ = (((uv >> 24) & 0x3f) | 0x80);
		*d++ = (((uv >> 18) & 0x3f) | 0x80);
		*d++ = (((uv >> 12) & 0x3f) | 0x80);
		*d++ = (((uv >>  6) & 0x3f) | 0x80);
		*d++ = (( uv        & 0x3f) | 0x80);
		return std::string(output);
	}

	// Corner case - returning codepoint 0x7fffffff representation
	// in UTF-8
        return std::string("\xfd\xbf\xbf\xbf\xbf\xbf");

}

// Converts a string to utf-8 using iconv
std::string Entities::convertUnicode(std::string s){
   iconv_t descriptor = iconv_open ("UTF-8", "ISO-8859-1");
   //char* original = (char*)malloc(s.length());
   char* original = (char*)s.c_str();
   char* input = original; 
   //cout << "Original = " << original << endl;
   size_t oldsize = s.length();
   size_t newsize= 6;
   size_t* outsize = &newsize;
   size_t* insize = &oldsize;
   char* converted = (char*)malloc(newsize);
   char* output = converted;
   //cout << "Converted.. ." << *converted << endl;
   memset (converted,' ',newsize);
   //cout << "NewSize: " << newsize << "\t Converted: " << *converted << endl;
   if (iconv(descriptor, &input, insize,
        &output, outsize) != -1){
      //cout << "Converted =" << *converted << "| Out: " << *outsize << endl;
      s = converted;
      int pos = s.find(" ");
      if (pos != std::string::npos) {
         s.erase(pos);
      }
   } else {
      //cout << "CONVERT ERROR!!!!" << "\n";
      s =  "error!";
   }
   iconv_close(descriptor);
   return s;
}

void Entities::initEntityHash(){
	entityHash["nbsp"] = 32;        // 160 would be unicode space, which can
	entityHash["iexcl"] = 161;       // cause troubles with "isspace". Use the
	entityHash["cent"] = 162;        // ascii space (32)
	entityHash["pound"] = 163;
	entityHash["curren"] = 164;
	entityHash["yen"] = 165;
	entityHash["brvbar"] = 166;
	entityHash["sect"] = 167;
	entityHash["uml"] = 168;
	entityHash["copy"] = 169;
	entityHash["ordf"] = 170;
	entityHash["laquo"] = 171;
	entityHash["not"] = 172;
	entityHash["shy"] = 173;
	entityHash["reg"] = 174;
	entityHash["macr"] = 175;
	entityHash["deg"] = 176;
	entityHash["plusmn"] = 177;
	entityHash["sup2"] = 178;
	entityHash["sup3"] = 179;
	entityHash["acute"] = 180;
	entityHash["micro"] = 181;
	entityHash["para"] = 182;
	entityHash["middot"] = 183;
	entityHash["cedil"] = 184;
	entityHash["sup1"] = 185;
	entityHash["ordm"] = 186;
	entityHash["raquo"] = 187;
	entityHash["frac14"] = 188;
	entityHash["frac12"] = 189;
	entityHash["frac34"] = 190;
	entityHash["iquest"] = 191;
	entityHash["Agrave"] = 192;
	entityHash["Aacute"] = 193;
	entityHash["Acirc"] = 194;
	entityHash["Atilde"] = 195;
	entityHash["Auml"] = 196;
	entityHash["Aring"] = 197;
	entityHash["AElig"] = 198;
	entityHash["Ccedil"] = 199;
	entityHash["Egrave"] = 200;
	entityHash["Eacute"] = 201;
	entityHash["Ecirc"] = 202;
	entityHash["Euml"] = 203;
	entityHash["Igrave"] = 204;
	entityHash["Iacute"] = 205;
	entityHash["Icirc"] = 206;
	entityHash["Iuml"] = 207;
	entityHash["ETH"] = 208;
	entityHash["Ntilde"] = 209;
	entityHash["Ograve"] = 210;
	entityHash["Oacute"] = 211;
	entityHash["Ocirc"] = 212;
	entityHash["Otilde"] = 213;
	entityHash["Ouml"] = 214;
	entityHash["times"] = 215;
	entityHash["Oslash"] = 216;
	entityHash["Ugrave"] = 217;
	entityHash["Uacute"] = 218;
	entityHash["Ucirc"] = 219;
	entityHash["Uuml"] = 220;
	entityHash["Yacute"] = 221;
	entityHash["THORN"] = 222;
	entityHash["szlig"] = 223;
	entityHash["agrave"] = 224;
	entityHash["aacute"] = 225;
	entityHash["acirc"] = 226;
	entityHash["atilde"] = 227;
	entityHash["auml"] = 228;
	entityHash["aring"] = 229;
	entityHash["aelig"] = 230;
	entityHash["ccedil"] = 231;
	entityHash["egrave"] = 232;
	entityHash["eacute"] = 233;
	entityHash["ecirc"] = 234;
	entityHash["euml"] = 235;
	entityHash["igrave"] = 236;
	entityHash["iacute"] = 237;
	entityHash["icirc"] = 238;
	entityHash["iuml"] = 239;
	entityHash["eth"] = 240;
	entityHash["ntilde"] = 241;
	entityHash["ograve"] = 242;
	entityHash["oacute"] = 243;
	entityHash["ocirc"] = 244;
	entityHash["otilde"] = 245;
	entityHash["ouml"] = 246;
	entityHash["divide"] = 247;
	entityHash["oslash"] = 248;
	entityHash["ugrave"] = 249;
	entityHash["uacute"] = 250;
	entityHash["ucirc"] = 251;
	entityHash["uuml"] = 252;
	entityHash["yacute"] = 253;
	entityHash["thorn"] = 254;
	entityHash["yuml"] = 255;
	entityHash["fnof"] = 402;
	entityHash["Alpha"] = 913;
	entityHash["Beta"] = 914;
	entityHash["Gamma"] = 915;
	entityHash["Delta"] = 916;
	entityHash["Epsilon"] = 917;
	entityHash["Zeta"] = 918;
	entityHash["Eta"] = 919;
	entityHash["Theta"] = 920;
	entityHash["Iota"] = 921;
	entityHash["Kappa"] = 922;
	entityHash["Lambda"] = 923;
	entityHash["Mu"] = 924;
	entityHash["Nu"] = 925;
	entityHash["Xi"] = 926;
	entityHash["Omicron"] = 927;
	entityHash["Pi"] = 928;
	entityHash["Rho"] = 929;
	entityHash["Sigma"] = 931;
	entityHash["Tau"] = 932;
	entityHash["Upsilon"] = 933;
	entityHash["Phi"] = 934;
	entityHash["Chi"] = 935;
	entityHash["Psi"] = 936;
	entityHash["Omega"] = 937;
	entityHash["alpha"] = 945;
	entityHash["beta"] = 946;
	entityHash["gamma"] = 947;
	entityHash["delta"] = 948;
	entityHash["epsilon"] = 949;
	entityHash["zeta"] = 950;
	entityHash["eta"] = 951;
	entityHash["theta"] = 952;
	entityHash["iota"] = 953;
	entityHash["kappa"] = 954;
	entityHash["lambda"] = 955;
	entityHash["mu"] = 956;
	entityHash["nu"] = 957;
	entityHash["xi"] = 958;
	entityHash["omicron"] = 959;
	entityHash["pi"] = 960;
	entityHash["rho"] = 961;
	entityHash["sigmaf"] = 962;
	entityHash["sigma"] = 963;
	entityHash["tau"] = 964;
	entityHash["upsilon"] = 965;
	entityHash["phi"] = 966;
	entityHash["chi"] = 967;
	entityHash["psi"] = 968;
	entityHash["omega"] = 969;
	entityHash["thetasym"] = 977;
	entityHash["upsih"] = 978;
	entityHash["piv"] = 982;
	entityHash["bull"] = 8226;
	entityHash["hellip"] = 8230;
	entityHash["prime"] = 8242;
	entityHash["Prime"] = 8243;
	entityHash["oline"] = 8254;
	entityHash["frasl"] = 8260;
	entityHash["weierp"] = 8472;
	entityHash["image"] = 8465;
	entityHash["real"] = 8476;
	entityHash["trade"] = 8482;
	entityHash["alefsym"] = 8501;
	entityHash["larr"] = 8592;
	entityHash["uarr"] = 8593;
	entityHash["rarr"] = 8594;
	entityHash["darr"] = 8595;
	entityHash["harr"] = 8596;
	entityHash["crarr"] = 8629;
	entityHash["lArr"] = 8656;
	entityHash["uArr"] = 8657;
	entityHash["rArr"] = 8658;
	entityHash["dArr"] = 8659;
	entityHash["hArr"] = 8660;
	entityHash["forall"] = 8704;
	entityHash["part"] = 8706;
	entityHash["exist"] = 8707;
	entityHash["empty"] = 8709;
	entityHash["nabla"] = 8711;
	entityHash["isin"] = 8712;
	entityHash["notin"] = 8713;
	entityHash["ni"] = 8715;
	entityHash["prod"] = 8719;
	entityHash["sum"] = 8721;
	entityHash["minus"] = 8722;
	entityHash["lowast"] = 8727;
	entityHash["radic"] = 8730;
	entityHash["prop"] = 8733;
	entityHash["infin"] = 8734;
	entityHash["ang"] = 8736;
	entityHash["and"] = 8743;
	entityHash["or"] = 8744;
	entityHash["cap"] = 8745;
	entityHash["cup"] = 8746;
	entityHash["int"] = 8747;
	entityHash["there4"] = 8756;
	entityHash["sim"] = 8764;
	entityHash["cong"] = 8773;
	entityHash["asymp"] = 8776;
	entityHash["ne"] = 8800;
	entityHash["equiv"] = 8801;
	entityHash["le"] = 8804;
	entityHash["ge"] = 8805;
	entityHash["sub"] = 8834;
	entityHash["sup"] = 8835;
	entityHash["nsub"] = 8836;
	entityHash["sube"] = 8838;
	entityHash["supe"] = 8839;
	entityHash["oplus"] = 8853;
	entityHash["otimes"] = 8855;
	entityHash["perp"] = 8869;
	entityHash["sdot"] = 8901;
	entityHash["lceil"] = 8968;
	entityHash["rceil"] = 8969;
	entityHash["lfloor"] = 8970;
	entityHash["rfloor"] = 8971;
	entityHash["lang"] = 9001;
	entityHash["rang"] = 9002;
	entityHash["loz"] = 9674;
	entityHash["spades"] = 9824;
	entityHash["clubs"] = 9827;
	entityHash["hearts"] = 9829;
	entityHash["diams"] = 9830;
	entityHash["quot"] = 34;
	entityHash["amp"] = 38;
	entityHash["lt"] = 60;
	entityHash["gt"] = 62;
	entityHash["OElig"] = 338;
	entityHash["oelig"] = 339;
	entityHash["Scaron"] = 352;
	entityHash["scaron"] = 353;
	entityHash["Yuml"] = 376;
	entityHash["circ"] = 710;
	entityHash["tilde"] = 732;
	entityHash["ensp"] = 8194;
	entityHash["emsp"] = 8195;
	entityHash["thinsp"] = 8201;
	entityHash["zwnj"] = 8204;
	entityHash["zwj"] = 8205;
	entityHash["lrm"] = 8206;
	entityHash["rlm"] = 8207;
	entityHash["ndash"] = 8211;
	entityHash["mdash"] = 8212;
	entityHash["lsquo"] = 8216;
	entityHash["rsquo"] = 8217;
	entityHash["sbquo"] = 8218;
	entityHash["ldquo"] = 8220;
	entityHash["rdquo"] = 8221;
	entityHash["bdquo"] = 8222;
	entityHash["dagger"] = 8224;
	entityHash["Dagger"] = 8225;
	entityHash["permil"] = 8240;
	entityHash["lsaquo"] = 8249;
	entityHash["rsaquo"] = 8250;
	entityHash["euro"] = 8364;
}

// Converts a single entity
std::string Entities::convertEntity(std::string entity){
   if (entity.length() == 0) {
      return entity;
   } else {
      if (entity[0] == '&') entity.erase(0,1);
      if (entity[0] == '#') {
         // TODO Totally different beast, yet to be implemented...
         entity.erase(0,1);
         unsigned int ient;
         if (tolower(entity[0]) == 'x') {
            // Convert to dec to make life easyer
            //ient = stoi(entity);
            from_string<unsigned int>(ient, entity, std::hex);
         } else {
            from_string<unsigned int>(ient, entity, std::dec);
         }
         entity = fromUnicodeToUTF(ient);
         //cout << "Convertido: " << entity << endl;
         return entity;
         //return Entities::convertUnicode(entity);
      } else {
         myHash::iterator it = entityHash.find(entity);
         //if (entityHash[entity] == NULL) {
         if (it == entityHash.end()) {
            // Do we know each other? Treat as a string...
            return entity;
         } else {
            // On the list. Just return it
            //std::cout << "ENTITY Hash: " << std::hex << entityHash[entity] 
            //   << std::endl;
            entity = fromUnicodeToUTF(entityHash[entity]);
            //std::cout << "ENTITY Hash conv: " << std::hex << entity 
            //   << std::endl;
            //cout << "Convertido: " << entity << endl;
            //return Entities::convertUnicode(entity);
            return entity;
            //std::string x = (std::string)it->second;
            //return Entities::convertUnicode(x);
         }
      }
   }
   // If everything else fails, treat it as a common string...
   return entity;
}


/* Will get the pages we acquired with crazy_collector
 * and extract all the tags from it. Must remember that 
 * (at least) the following tags must have the content 
 * between their open-close tags ignored: textarea, 
 * script, comments and style.
 */
#include "gzstream.h"
#include "zfilebuf.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto
#include "boost/filesystem/path.hpp"       // ditto
#include <algorithm>
#include <ext/hash_map>
#include "entities.h"
#include "utils.h"

// The last docId we have
#define LAST_DOC 1244550
#define DIR_SIZE 10000

using namespace std;
namespace bfs=boost::filesystem;

//myHash entityHash;

Entities* E;

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

void createDir(string path){
   int pos = path.rfind("/");
   path.erase(pos);
   bfs::create_directory(bfs::path(path));
}

// FIXME Search for "&; chars" and substitute them for normal chars
void convertSpecialChars(string& output){
   int beginPos = output.find("&");
   int endPos;
   string tmp = "", newStr;
   while (beginPos != string::npos) {
      endPos = output.find(";",beginPos);
      if (endPos != string::npos){
         newStr = "";
         tmp = "";
         for (int i = beginPos; i < endPos; ++i) {
            if (output[i] == '&' || isspace(output[i])) {
               // Start of a new entity or closing of one. 
               // Send the prior (if any)
               // collected entity to decode and prepare
               // to collect a new one
               if (tmp != "") {
                  //cout << "Entity: " << tmp << endl;
                  newStr += E->convertEntity(tmp);
                  tmp = output[i];
                  //tmp = "";
               } else {
                  // must be just a whitespace. Print it anyway...
                  if (isspace(output[i])) newStr += " ";
               }
            } else {
               tmp += output[i];
            }
         }
         //cout << "Entity: " << tmp << endl;
         newStr += E->convertEntity(tmp);
         std::transform(newStr.begin(), newStr.end(), 
               newStr.begin(), (int(*)(int)) tolower);
         // Making the substitution
         output.erase(beginPos,endPos-beginPos+1);
         output.insert(beginPos,newStr);
         /*
         ofstream out("entities.txt", fstream::app);
         out << "File: " << file << "\t\t\t" <<  token << endl;
         out.close();
         */
      } else {
         // This really shouldn't happend, but just in case...
         // Knowing we won't find any ";" closing an entity, 
         // just return from here
         return;
      }
      // We will start the new search from the same beginPos
      // because we have erased part of the string. it will 
      // be safer to start from here.
      beginPos = output.find("&",beginPos);
   }
}

// Finds the given endtag and returns the next position to parse
int findEndTag(memory_buffer& dec,int pos, string endtag) {
   char et = endtag[0];
   bool inquote = false;
   string tmp;
   while (pos < dec.second) {
      if (inquote && 
            (dec.first[pos] != '\"')){
         ++pos;
      } else if (!inquote && 
            (dec.first[pos] == '\"')){
         inquote = true;
         ++pos;
      } else if (inquote && 
            (dec.first[pos] == '\"')){
         inquote = false;
         ++pos;
      } else {
         if (tolower(dec.first[pos]) == et) {
            // May be our end tag...
            tmp.assign(&dec.first[pos],endtag.length());
            std::transform(tmp.begin(), tmp.end(), 
               tmp.begin(), (int(*)(int)) tolower);
            if (tmp == endtag) {
               // Found it! Good to go.
               return pos+endtag.length();
            } else {
               // No go... Next!
               ++pos;
            }
         } else {
            ++pos;
         }
      }
   }
   // If it got here, we have an unfinished tag...
   return dec.second+1;
}

// Parse a file to remove any tags. Will also treat &; chars
void parseFile(string file){
   // decompressing the file
   memory_buffer dec(0,0);
	try{
		dec = decompres(file);
	} catch(...) {
	   // pass
	}
   //cout << dec.first << endl;
   // create destiny directory and output file
   string myOutFile = file;
   myOutFile.erase(0,24);
   myOutFile = "stripped" + myOutFile;
   createDir(myOutFile);
   ogzstream outFile(myOutFile.c_str());
   string token = "", output = "";
   bool wspace = true;
   int i = 0;
   // Will return a token, i. e., a string that will stop
   // at the first space found
   while (i < dec.second) {
      if (dec.first[i] != '<') {
         // Normal text
         if (wspace && (isspace(dec.first[i]))){
            // We will not repeat whitespaces!
         } else if (wspace && (not isspace(dec.first[i]))){
            wspace = false;
            output += tolower(dec.first[i]);
         } else if (not wspace && (isspace(dec.first[i]))){
            wspace = true;
            output += tolower(dec.first[i]);
         } else {
            output += tolower(dec.first[i]);
         }
         i++;
      } else {
         // Test to verify if it is a "special tag
         token.assign(&dec.first[i],9);
         std::transform(token.begin(), token.end(), 
               token.begin(), (int(*)(int)) tolower);
         if (token.compare(0,4,"<!--") == 0){
            i = findEndTag(dec, i+1, "-->");
         } else if (token.compare(0,9,"<textarea") == 0) {
            i = findEndTag(dec, i+1, "</textarea>");
         } else if (token.compare(0,6,"<style") == 0) {
            i = findEndTag(dec, i+1, "</style>");
         } else if (token.compare(0,7,"<script") == 0){
            i = findEndTag(dec, i+1, "</script>");
         } else {
            // Nope, just a common tag. Remove it
            i = findEndTag(dec, i+1,">");
         }
      }
   }
   //cout << output << endl;
   // Entity conversion
   convertSpecialChars(output);
   //cout << "CONVERTED!\n" << output << endl;
   //outFile << output;
   // releasing memory
	delete[] dec.first;
   outFile.close();
}

int main(){
   unsigned long int docId = 0;
   //docId = 675705;
   string file = getNextFile(docId);
   E = new Entities;
   //file = "newtest.gz";
   while (file != ""){
      parseFile(file);
      file = getNextFile(docId);
   }
}

/* Class responsible for extracting any links in a webpage we collect.
 * If the not textual elements (i.e. tags) are to be removed, it will
 * do this service. Probably will save the page. Also probably it will
 * store the links for each page, so that we can use pagerank in the
 * search step of the project.
 */
#include <set>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "parser.h"

using namespace std;

// Remove all anchor texts from urls (everything after a #)
string removeAnchor(string url){
   int pos = url.find("#");
   if (pos <= url.length()-1) {
      return url.erase(pos);
   } else return url;
}

// Returns the base domain
string parser::domainName(string url){
   // Must start with http://
   int pos = url.find("/", 8);
   if (pos <= url.length()-1){
      url.erase(pos);
   }
   // Only the domain remains
   return url;
}

// Returns true if the url leads to a directory or to
// a web page (.htm, .html, .php, etc)
bool goodToDownload(string url){
   int pos = url.find("/");
   while(pos < url.length()-1){
      url.erase(0,pos+1);
      pos = url.find("/");
   }
   pos = url.find(".");
   if (pos > url.length()-1){
      // leads to a directory
      return 1;
   } else if (url.find(".",pos+1) < url.length()-1){
      return 1;
   } else {
      if (url.compare(pos,4,".htm")   == 0 ||
          url.compare(pos,5,".html")  == 0 ||
          url.compare(pos,6,".dhtml") == 0 ||
          url.compare(pos,6,".shtml") == 0 ||
          url.compare(pos,6,".xhtml") == 0 ||
          url.compare(pos,6,".jhtml") == 0 ||
          url.compare(pos,4,".php")   == 0 ||
          url.compare(pos,4,".asp")   == 0 ||
          url.compare(pos,5,".aspx")  == 0 ||
          url.compare(pos,4,".jsp")   == 0 ||
          url.compare(pos,4,".cfm")   == 0 ||
          url.compare(pos,5,".cfml")  == 0 ){
         return 1;
      } else {
         return 0;
      }
   }
}

/*
 * Returns a URL in the absolute and canonical form (no trailing "/").
 * For now it will consider dynamic pages "as is".
 * Only http will be considered.
 * Javascript will be ignored.
 */
string treatLink(string link, string base){
   if (link.compare(0,11,"javascript:") == 0){
      // Javascript, so ignore.
      return "";
   } else if (link.compare(0,7,"mailto:") == 0){
      // mailto, so ignore.
      return "";
   } else if (link.compare(0,7,"http://") == 0){
      //It's a common url. Just erase trailing /
      if (link[link.length()-1] == '/'){
         link.erase(link.length()-1, 1);
      }
      //if (is_br(link) && goodToDownload(link)){
      if (goodToDownload(link)){
         return removeAnchor(link);
      } else return "";
   } else if (link.find("://") < link.length()){
      // Another protocol (ftp, https, etc). We will ignore it.
      return "";
   } else {           // Anything else should be relative links
      /* If my base url links to a file, I must disconsider it
       * while looking for directories. It Will use a siple heuristic:
       * if there is more than 1 "." in the string, it will consider
       * the substring a domain name (because there should be at least 2
       * dots in this case (for example www.google.com)), or else it's a file.
       */
      if (base == ""){
         // This can happend if some douchebag put a 
         // <base href="">. And yes, it has happened...
         return "";
      }
      string root = parser::domainName(base);
      base = base.erase(0,root.length());
      if (base[0] == '/') base.erase(0,1);
      int pos;
      pos = base.find(".");
      // If url points to a file
      if (pos < base.length()-1){
         pos = base.rfind("/");
         // File at the root of the domain
         if (pos < 0){ base = "";} 
         else { base.erase(pos);}
      }
      // Considering ../, ./ and //
      while (link.compare(0,3,"../") == 0 ||
             link.compare(0,2,"./")  == 0 ||
             link.compare(0,2,"//")  == 0 ||
             link.compare(0,1,"/")   == 0 ||
             link[0] == '\r' || 
             link[0] == '\n' ||
             link[0] == ' '){
         if (link.compare(0,3,"../") == 0){
            if (base != ""){
               pos = base.rfind("/");
               if (pos < 0){ base = "";} 
               else { base.erase(pos);}
            }
            link.erase(0,3);
         } else if (link.compare(0,2,"./") == 0){
            link.erase(0,2);
         } else if (link.compare(0,2,"//") == 0){
            link.erase(0,2);
         } else {
            link.erase(0,1);
         }
               
      }
      link =  base + "/" + link;
      // Just for good measure...
      if (link[link.length()-1] == '/') link.erase(link.length()-1); 
      if (link[0] == '/') link.erase(0,1);
      root = root + "/" + link;
      //if (is_br(root) && goodToDownload(root)){
      if (goodToDownload(root)){
         return removeAnchor(root);
      } else return "";
   }
   return "WTF?";
   
   // size_type find_first_of(const basic_string& s, size_type pos = 0) const
   // pode ajudar a verificar a raiz do dominio...
}

bool inline addx(int* x, int plus, int limit){
   *x += plus;
   return (*x > limit);
}


set<string> parser::getLinks(memory_buffer page, string base){
   set<string> links;
   int i = 0;
   int x;
   string lnk = "";
   string enc = "";
   string tmp;
   bool index = true;
   bool follow = true;
   while (i < page.second){
      /*
       * To find links. Must be run only if not in a comment, not
       * between "" or any other uneligible conditions.
       */
      x = i;
      if (page.first[x] == '<'){x++; goto inittag;} else {goto end;}
      inittag: if (tolower(page.first[x]) == 'a'){
            if (addx(&x,1,page.second)) {goto escape;}
            goto a;
         } else if (tolower(page.first[x]) == 'i'){
            if (addx(&x,1,page.second)) {goto escape;}
            goto frame;
         } else if (tolower(page.first[x]) == 'f'){
            goto frame;
         } else if (page.first[x] == '!'){
            goto quote;
         } else if (tolower(page.first[x]) == 's'){
            goto script;
         } else if (tolower(page.first[x]) == 'm'){
            goto meta;
         } else if (tolower(page.first[x]) == 'b'){
            goto b;
         } else goto endtag;
      
      b: tmp.assign(&page.first[x], 5);
         std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
          (int(*)(int)) tolower);
         if (tmp == "base "){
            if (addx(&x,5,page.second)) goto escape;
            goto base;
         } else goto endtag;
         
      base: while (tolower(page.first[x]) != 'h' && 
                page.first[x] != '>'){
               if (addx(&x,1,page.second)) {goto escape;}
            }
            if (tolower(page.first[x]) == 'h') {
               tmp.assign(&page.first[x], 4);
               std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
                (int(*)(int)) tolower);
               if (tmp == "href"){
                  if (addx(&x,4,page.second)) goto escape;
                  while (isspace(page.first[x])){
                     if (addx(&x,1,page.second)) {goto escape;}
                  }
                  if (page.first[x] == '='){
                     if (addx(&x,1,page.second)) {goto escape;}
                     while (isspace(page.first[x])){
                        if (addx(&x,1,page.second)) {goto escape;}
                     }
                     goto hl2;
                  } else {goto escape;}
               } else {
                  if (addx(&x,1,page.second)) goto escape;
                  goto base;
               }
            } else goto endtag;

      hl2: if (page.first[x] == '\'' || page.first[x] == '\"'){
              if (addx(&x,1,page.second)) goto escape;              
           }
           base = "";
           while (page.first[x] != isspace(page.first[x]) &&
                  page.first[x] != '\'' &&
                  page.first[x] != '\"'){
              base += page.first[x];
              if (addx(&x,1,page.second)) goto escape;
           }
           goto endtag;
               
      meta: tmp.assign(&page.first[x], 5);
            std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
             (int(*)(int)) tolower); // for case-insensitive matching
            if (tmp == "meta "){
               if (addx(&x,5,page.second)) goto escape;
               goto encoding;               
            } else {
               goto endtag;
            }
      encoding: while (tolower(page.first[x]) != 'c' 
                      && page.first[x] != '>'
                      && tolower(page.first[x]) != 'n'){
                  if (addx(&x,1,page.second)) goto escape;
                }
                if (tolower(page.first[x]) == 'n'){
                   tmp.assign(&page.first[x],4);
                   std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
                    (int(*)(int)) tolower);
                   if (tmp == "name"){
                      if (addx(&x,4,page.second)) goto escape;
                         while (isspace(page.first[x]) || page.first[x] == '='){
                           if (addx(&x,1,page.second)) goto escape;
                      }
                      if (page.first[x] != '\"' || 
                          page.first[x] != '\''){
                         if (addx(&x,1,page.second)) goto escape;
                      } else if (page.first[x] == '>') goto endtag;
                      tmp.assign(&page.first[x],6);
                      std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
                        (int(*)(int)) tolower);
                      if (tmp == "robots"  ||
                          tmp == "robots" ||
                          tmp == "robots"){
                         if (addx(&x,6,page.second)) goto escape;
                         goto robots;
                      } else {
                         if (addx(&x,1,page.second)) goto escape;
                         goto encoding;
                      }
                   } else {
                      if (addx(&x,1,page.second)) goto escape;
                      goto encoding;
                   }
                } else if (tolower(page.first[x]) == 'c'){
                   tmp.assign(&page.first[x], 7);
                   std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
                    (int(*)(int)) tolower);
                   if (tmp == "charset"){
                      if (addx(&x,7,page.second)) goto escape;
                      while (isspace(page.first[x]) || page.first[x] == '='){
                         if (addx(&x,1,page.second)) goto escape;
                      }
                      while (page.first[x] != '\"' && 
                             page.first[x] != '\'' &&
                             page.first[x] != ';'  &&
                             !isspace(page.first[x])) {
                         enc += page.first[x];
                         if (addx(&x,1,page.second)) {goto escape;}
                      }
                      //cout << "THE ENCODING IS: " << enc << "\n";
                      goto endtag;
                   } else {
                      if (addx(&x,1,page.second)) goto escape;
                      goto encoding;
                   }
                } else goto endtag;

      robots: while (tolower(page.first[x]) != 'c' &&
                      page.first[x] != '>'){
                  if (addx(&x,1,page.second)) goto escape;
              }
              if (page.first[x] == '>'){
                 goto endtag;
              } else {
                 tmp.assign(&page.first[x], 7);
                 std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
                  (int(*)(int)) tolower);
                 if (tmp == "content"){
                    if (addx(&x,7,page.second)) goto escape;
                    while (isspace(page.first[x]) || page.first[x] == '='){
                       if (addx(&x,1,page.second)) goto escape;
                    }
                    tmp = "";
                    if (page.first[x] != '\"' || 
                        page.first[x] != '\''){
                       // if has quotes, can have more then one command
                       if (addx(&x,1,page.second)) goto escape;
                       while (page.first[x] != '\"' && 
                              page.first[x] != '\'' &&
                              page.first[x] != '>'){
                          if (page.first[x] == ','){
                             // We've got a command.Evaluate!
                             //evaluateRobots(tmp, &index, &follow);
                             tmp = "";
                          } else {
                             tmp += page.first[x];
                          }
                          if (addx(&x,1,page.second)) {goto escape;}
                       }
                       //evaluateRobots(tmp, &index, &follow);
                       // evaluated the robots meta tag. If it is
                       // a nofollow, stop the parsing now
                       if (follow){
                          //cout << "Going to parse!\n";
                          goto endtag;
                       } else {
                          //cout << "For some reason, not parsing...\n";
                          goto kill_parsing;
                       }
                       if (addx(&x,1,page.second)) {goto escape;}
                       goto endtag;
                    } else if (page.first[x] == '>') {
                       goto endtag;
                    } else {
                       // just one command
                       while (page.first[x] != '>' &&
                              !isspace(page.first[x])){
                          tmp += page.first[x];
                          if (addx(&x,1,page.second)) {goto escape;}
                       }
                       // Evaluate!
                       //evaluateRobots(tmp, &index, &follow);
                       // evaluated the robots meta tag. If it is
                       // a nofollow, stop the parsing now
                       if (follow){
                          //cout << "Going to parse!\n";
                          goto endtag;
                       } else {
                          //cout << "For some reason, not parsing...\n";
                          goto kill_parsing;
                       }
                       goto endtag;
                    }  
                 } else {
                    if (addx(&x,1,page.second)) goto escape;
                    goto robots;
                 }
              }
                   
      script: tmp.assign(&page.first[x], 7);
              //cout << "Maybe a \"script\" at position " << x << 
              //   ", it is, in fact a " << tmp << endl;
              std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
               (int(*)(int)) tolower); // for case-insensitive matching
              //cout << "Lowercase successful: " << tmp << endl;
              if (tmp == "script>"){
                 if (addx(&x,7,page.second)) {goto escape;}
                 goto endscript;
              } else goto endtag;
      endscript: while (page.first[x] != '<'){
                    if (addx(&x,1,page.second)) goto escape;
                 }
                 tmp.assign(&page.first[x],9);
                 std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
                  (int(*)(int)) tolower);
                 if (tmp == "</script>"){
                    if (addx(&x,8,page.second)) {goto escape;}
                    goto endtag;
                 } else {
                    if (addx(&x,1,page.second)) {goto escape;}
                    goto endscript;
                 }
      quote: if (page.first[x]   == '!' &&
                 page.first[x+1] == '-' &&
                 page.first[x+2] == '-'){
                if (addx(&x,3,page.second)) {goto escape;}
                goto endquote;
             } else goto endtag;
      
      endquote: while (page.first[x] != '-'){
                   x++;
                   if (x > page.second) {i = x; goto escape;}
                }
                if (page.first[x]   == '-' && 
                    page.first[x+1] == '-' && 
                    page.first[x+2] == '>'){
                   if (addx(&x,2,page.second)) {goto escape;}
                   goto endtag;
                } else {
                   if (addx(&x,1,page.second)) {goto escape;}
                   goto endquote;
                }
             
      frame: if (tolower(page.first[x]) == 'f'){
                tmp.assign(&page.first[x], 5);
                //cout << "Maybe a \"frame\" at position " << x << 
                // ", it is, in fact a " << tmp << endl; 
                std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
                  (int(*)(int)) tolower); // for case-insensitive matching
                //cout << "Lowercase successful: " << tmp << endl;
                if (tmp == "frame"){
                   if (addx(&x,5,page.second)) {goto escape;}
                   goto space3;
                } else goto endtag;
             } else goto endtag;
      space3: if (isspace(page.first[x])){x++; goto src;} else {goto endtag;}
              
      src: while (isspace(page.first[x]) ||
                (tolower(page.first[x]) != 's' &&
                page.first[x] != '>')) {x++;}
           if (tolower(page.first[x]) == 's'){
              tmp.assign(&page.first[x], 3);
              std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
                (int(*)(int)) tolower); // for case-insensitive matching
              if (tmp == "src"){
                 if (addx(&x,3,page.second)) {goto escape;}
                 goto space2;
              } else {
                 if (addx(&x,1,page.second)) {goto escape;}
                 goto src;
              }
           } else if (page.first[x] == '>'){
              i = x;
              goto end;
           } else {
              if (addx(&x,1,page.second)) {goto escape;}
              goto space3;
           }
              
      a: if (isspace(page.first[x])){x++; goto space;} else {goto endtag;}
      space: while (isspace(page.first[x]) || 
                  (tolower(page.first[x]) != 'h' &&
                  page.first[x] != '>')) {
                if (addx(&x,1,page.second)) {goto escape;}
             }
             if (tolower(page.first[x]) == 'h'){
                tmp.assign(&page.first[x], 4);
                std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
                  (int(*)(int)) tolower); // for case-insensitive matching
                if (tmp == "href"){
                   if (addx(&x,4,page.second)) {goto escape;}
                   goto space2;
                } else {
                   if (addx(&x,1,page.second)) {goto escape;}
                   goto space;
                }
             } else if (page.first[x] == '>'){
                i = x;
                goto end;
             } else {
                if (addx(&x,1,page.second)) {goto escape;}
                goto space;
             }
      space2: if (isspace(page.first[x])) {
            while (isspace(page.first[x])){
               if (addx(&x,1,page.second)) {goto escape;}
            } 
            goto eq;
         } else {goto eq;}
      eq: if (page.first[x] == '='){
             if (addx(&x,1,page.second)) {goto escape;}
             if (isspace(page.first[x])) {
               while (isspace(page.first[x])){
                  if (addx(&x,1,page.second)) {goto escape;}
               }
               goto hl;
             } else {goto hl;}
          } else { 
             if (addx(&x,1,page.second)) {goto escape;}
             goto space;
          }
      hl: if (page.first[x] == '\"' || page.first[x] == '\''){
             if (addx(&x,1,page.second)) {goto escape;}
             while (page.first[x] != '\"' && page.first[x] != '\'') {
                lnk += page.first[x];
                if (addx(&x,1,page.second)) {goto escape;}
             }
             if (lnk != ""){ 
               lnk = treatLink(lnk, base);
               links.insert(lnk);
               //cout << "The link is: "<< lnk << "\n";
               lnk = "";
             }
             goto endtag;
          } else {
             while (!isspace(page.first[x]) && page.first[x] != '>') {
                lnk += page.first[x];
                if (addx(&x,1,page.second)) {goto escape;}
             }
             if (lnk != ""){ 
               lnk = treatLink(lnk, base);
               //cout << "The link is: " << lnk << "\n";
               links.insert(lnk);
               lnk = "";
             }
             goto endtag;
          }
      endtag: while (page.first[x] != '>') {
                 if (addx(&x,1,page.second)) goto escape;
              } 
              i = x; 
              goto end;
      escape: i = x; goto end; 
      end: i++; 
   }
   kill_parsing:;
   // Saving the page (utf-8). If no charset was found in the page,
   // consider it iso-8859-1 (latin-1) as default. Only if can index
   /*
   if (index){
   //   cout << "Going to index!\n";
      savePage(page, enc, base);
   }
   */
   // Return the set with the collected links
   return links;
}

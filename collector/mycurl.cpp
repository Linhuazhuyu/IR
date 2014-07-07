/*
 * Thread class. Will be responsible for getting the pages (with libcurl).
 * Maybe it will also store the pages. Probably will call the parser class,
 * but it can change if we use 2 kinds of threads (not likely).
 */

#include <fstream>
#include "mycurl.h"
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <iostream>

struct PageStruct {
  char *memory;
  size_t size;
  std::string myUrl;
};


void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}
 
size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct PageStruct *mem = (struct PageStruct *)data;
  mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

/*
 * Initializes curl. To be called at the start of the program.
 */
void mycurl::curlInit(){
  curl_global_init(CURL_GLOBAL_ALL);
}

/*
 * Frees the memory of a PageStruct
 */
void mycurl::clearChunk(PageStruct ms){
   if(ms.memory)
      free(ms.memory);
}

PageStruct mycurl::getPage(const std::string page){
  CURL *curl_handle;

  struct PageStruct chunk;

  chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
  chunk.size = 0;    /* no data at this point */

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, page.c_str());

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  
  /* to follow redirects */
  curl_easy_setopt (curl_handle, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt (curl_handle, CURLOPT_MAXREDIRS, 10);
     
  /* to avoid problems with dns timeouts */
  curl_easy_setopt (curl_handle, CURLOPT_NOSIGNAL, 1);
  
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "crazy_crawler/1.0");

  /* Set Timeout (15) */
  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 15);

  /* Verbose. Just for testing */
  //curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1);
    
  /* get it! */
  curl_easy_perform(curl_handle);

  /* get error code*/
  long httpCode;
  curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &httpCode );

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  /*
   * Now, our chunk.memory points to a memory block that is chunk.size
   * bytes big and contains the remote file.
   * Also, verify for error codes. 
   */
   
  if(httpCode != 200){
    chunk.size = 0;
  } 
  return chunk;
}

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ext/hash_map>
#include <tr1/functional>
#include <string>
#include <vector>
#include "tuples.h"


const std::string CRLF =  "\x0d\x0a";
const std::string CRLF_CRLF =  "\x0d\x0a\x0d\x0a";

typedef __gnu_cxx::hash_map < std::string, std::string,
                        std::tr1::hash<std::string> > StrStrMap;

class HttpClientHandler {
   public:
      int fd;
      struct sockaddr_in cli_addr; 
	   std::string method;	  //!< The HTTP method used by this client.
	   std::string uri;	  //!< The URI used in this client's request.
	   std::string proto_version;// HTTP protocol version used in the request.
      std::vector<std::string> parseRequest();
      StrStrMap headers;
      HttpClientHandler(int cli_fd);
      void closeConnection();
      void write(const std::string data);
};

class HttpServer {
   private:
      int setupServerSocket(uint16_t server_port, int backlog);
      void handleNewClient(int cli_fd, struct sockaddr_in cli_addr);
      int server_fd; //!< Server socket file-descriptor.
   public:
      HttpServer();
      HttpClientHandler run();
};

// Helper functions
std::string mkResultPage(std::vector<ResultTuple> results);

std::string mk_response_header(std::string msg="OK", int code=200,
		std::string type="text/html; charset=utf-8",
		std::string extra_hdrs="");

//void mkResultsFragment(const vec_res_vec_t& matches, std::string& results);

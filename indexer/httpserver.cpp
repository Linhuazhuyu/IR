#include <exception>
#include <iostream>
#include <algorithm>
#include "httpserver.h"
#include "utils.h"
#include <ctype.h>

//! Parse percent encoded data
std::string decodePE(std::string msg) {
   using std::istringstream;
   std::string result;
   size_t pos = 0;
   while( pos < msg.size() ) {
      if (msg[pos] == '+') {
         result += ' ';
          ++pos;
      } else if (msg[pos] == '%') {
         if (pos + 2 < msg.size() &&
             isxdigit(msg[pos+1]) &&
             isxdigit(msg[pos+2])) {
            char c;
            size_t value;

            istringstream in(msg.substr(pos+1,2));
            in >> std::hex >> value;
            c = value;

            result += c;
            pos += 3;
         } else {
            result += '%';
            ++pos;
         }
      } else {
         size_t len = msg.find_first_of("+%",pos) - pos;
         result += msg.substr(pos, len);
         pos += len;
      }
   }
   return result;
}

int HttpServer::setupServerSocket(uint16_t server_port, int backlog = 10){
	int fd = -1;
	int res = 0;
	int yes = 1;
	struct sockaddr_in addr;

	/* Socket creation */
	fd = socket(AF_INET, SOCK_STREAM,0);
        if (fd < 0) {
                throw "Error creating listening socket.";
        }

        /* lose the pesky "Address already in use" error message */
        if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
                throw "Error configuring socket: SO_REUSEADDR can't be set!";
        }

	/* Bind to listening port */
	bzero(&addr, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(server_port);
	addr.sin_addr.s_addr = INADDR_ANY;
	res = bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)); 
	if (res){
		throw "Error binding to listening port.";
	}

	/* Listen for connections on socket */
	if (listen(fd,backlog) == -1) {
		throw "Error listening";
	}
   return fd;
}


HttpServer::HttpServer() {
   server_fd = setupServerSocket(8090);
}

HttpClientHandler HttpServer::run(){
	int cli_fd;
	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len = sizeof(struct sockaddr);

	cli_fd = accept(server_fd, (struct sockaddr*) &cli_addr,&cli_addr_len);
	if (-1 == cli_fd ){
		throw "Error in accept.";
	}
	return HttpClientHandler(cli_fd);
}

//////////////////////////////////////////////////////

HttpClientHandler::HttpClientHandler(int cli_fd){
   fd = cli_fd; 
}

std::vector<std::string> HttpClientHandler::parseRequest(){
   const int BUF_LEN = 1024;
   char buf[BUF_LEN];
   ssize_t n;
   std::string msg;

   // Read until the end of the request
   while( 0 < (n = recv(fd,buf, BUF_LEN, 0)) ) {
      msg.append(buf, n);
      // Get out if found the request end
      if (msg.find(CRLF_CRLF)) break;
   }

   // parse request
   if (msg.empty()) {
      throw "Bad Request HTTP Exception";
   }
   
   std::vector<std::string> fullreq_lines(split(msg,CRLF));
   std::vector<std::string>::iterator line = fullreq_lines.begin();
   std::vector<std::string> request_fields(split(*line," ",2));
   if (request_fields.size() != 3) {
      throw "Bad Request HTTP Exception";
   }
   method = request_fields[0];
   std::string uri = request_fields[1];
   proto_version = request_fields[2];
   // Get the query!!
   std::transform(uri.begin(), uri.end(), 
            uri.begin(), (int(*)(int)) tolower);
   std::cout << "URI -> " << uri << std::endl;
   int pos = uri.find("/?q=");
   if (pos == std::string::npos) {
      std::cout << "NOT Query!\n";
      std::vector<std::string> q;
      if (uri.find("/cache/") != std::string::npos) {
         q.push_back(uri);
      }
      return q;
   } else {
      std::cout << "Query!\n";
      uri.erase(0, pos+4);
   }
   pos = uri.find("&");
   if (pos != std::string::npos) {
      uri.erase(pos);
   }
   uri = decodePE(uri);
   std::vector<std::string> q_terms(split(uri," "));
   return q_terms;
}

void HttpClientHandler::write(const std::string data){
	const char*  buf = data.c_str();
	const size_t len = data.size();

	size_t total = 0;		// how many bytes we've sent
	size_t bytesleft = len;		// how many we have left to send
	int n;

	while(total < len) {
		n = send(fd, buf+total, bytesleft, 0);
		if (n == -1) {
			throw "Error in send()";
		}
		total += n;
		bytesleft -= n;
	}

}

void HttpClientHandler::closeConnection(){
	if(fd) {
		if( -1 == close(fd) ){
			throw "Error in close()";
		}
		fd = 0;
	}
}

///////////////////////////////////////////////
std::string mk_response_header(std::string msg, int code,
			std::string type, std::string extra_hdrs){
	std::ostringstream buf;
	//const std::string& CRLF = http::CRLF;

	buf <<	"HTTP/1.0 " << code <<  msg << CRLF <<
		"Server: ProcastinationBroadcaster/0.1" << CRLF <<
		"Connection: close" << CRLF << 
		"Content-Type: " << type << CRLF << 
		extra_hdrs <<  // They should contain line-ending CRLFs
		CRLF;
	
	return buf.str();
}

std::string mkResultPage(std::vector<ResultTuple> results)	{
   std::string title;
   std::string q_val;

   std::ostringstream out;
   out <<  "<html>\n"
      "<head>\n"
      "<title>" << title << "Pergunte ao Jarbas.</title>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
      "</head>\n"
      "<body>\n"
      "<h1>Pergunte ao Jarbas</h1>\n"
      "<h2>Pra onde, chefe?</h2>\n"
      "<form action=\"/\" method=\"get\" >\n"
      "<input name=\"q\" type=\"text\" id=\"q2\" size=\"25\" value=\"" << q_val <<  "\">\n"
      "<input type=\"submit\" name=\"Submit\" value=\"Enviar\">\n"
      "</form>\n";
      //results
      if (results.size() > 0) {
         out << "<table border=\"0\">\n";
         std::vector<ResultTuple>::iterator it = results.begin();
         int i = 0;
         while (it != results.end() && i < 100) {
            out << "<tr>\n<td><a href=\"cache/" << it->doc_id <<
              "\">" << it->doc_id << "</a></td><td>"
               << it->similarity << "</td>\n</tr>\n";
            ++it;
            ++i;
         }
         out << "</table>\n";            
      } 
      out << "</body>\n"
      "</html>\n" << std::endl;

   return out.str();
}

#include <stdexcept>
#include <string>

typedef std::pair<char*,int> memory_buffer;

/*
class GZFileWrapperException: public std::runtime_error {
public:
	GZFileWrapperException (std::string msg="")
                :runtime_error(msg){}
};


class GZFileWrapper {
public:
	gzFile fh;
	GZFileWrapper(const char* filename, const char* mode = "r")
	: fh(NULL);
   ~GZFileWrapper();
   bool eof();
   int read(char* buf, unsigned len);
};
*/

memory_buffer decompres(const std::string filename);

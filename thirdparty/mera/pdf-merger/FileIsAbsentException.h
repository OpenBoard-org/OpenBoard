#if !defined FileIsAbsentException_h
#define FileIsAbsentException_h

#include <string>

class FileIsAbsentException
{
public:
    FileIsAbsentException(const char * fileName):
    _fileName(fileName){};
    const char * getFileName()
    {
        return _fileName.c_str();
    }
private:
    std::string _fileName;
};

#endif

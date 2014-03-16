#pragma once

class myStringArray
{
    public:
        const int length;
        std::string* strArray;

        myStringArray(int l = 0) : length(l)
        {
            strArray = new std::string[length];
        }
};

myStringArray splitString(std::string &s, char ch);

char* genResponse(const std::string &responseCode, const std::string &ContentType, int ContentLength, const char * data, int * length);
char* genResponseError(const std::string &responseCode, const std::string &ContentType, int * length);
char* genResponseMovedPermanently(const std::string &path, int * length);

char * ReadAllBytes(char const * filename, int * read, bool * fileFound);

std::string getHost(std::string &hostLine);
std::string getPath(std::string &getLine);
std::string getType(std::string &path);

bool getConnectionClose(std::string &connectionLine);
bool checkPath(std::string &path);
bool checkCatalog(std::string &path);



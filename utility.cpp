#include <sstream> // int to string
#include <fstream> // file read
#include <iostream>
#include <string>
#include "utility.h"

using namespace std;

myStringArray splitString(std::string &s, char ch)
{
    const int maxLines = 30;

    std::string tempArray[maxLines];

    unsigned int index = 0;
    int id = 0;
    unsigned int pre_id = 0;

    while( (id = s.find(ch ,pre_id))>=0 )
    {
        std::string temp = s.substr(pre_id, id-pre_id);
        tempArray[index] = temp;
        index++;

        pre_id = id + 1;
    }

    if(pre_id < s.length()) {
        std::string temp = s.substr(pre_id);

        tempArray[index] = temp;
        index++;
    }

    const int length = index;
    myStringArray toReturn(length);

    for(int i=0; i<length; i++)
    {
        toReturn.strArray[i] = tempArray[i];
    }

    return toReturn;
}

char* genResponse(const std::string &responseCode, const std::string &ContentType, int ContentLength, const char * data, int * length)
{
    std::string Response = "HTTP/1.1 ";
    Response += responseCode + "\n";

    ostringstream ss;
    ss << ContentLength;
    //Response += "Accept-Ranges: bytes\n";
    Response += "Content-Length: " + ss.str() + "\n";
    Response += "Content-Type: " + ContentType + "\n\n";

    *length = Response.length() + ContentLength;
    char* buff = new char [Response.length() + ContentLength+1];

    for(unsigned int i=0; i<Response.length(); ++i)
    {
        buff[i] = Response[i];
    }

    for(int i=0; i<ContentLength; ++i)
    {
        buff[Response.length()+i] = data[i];
    }

    return buff;
}

char* genResponseError(const std::string &responseCode, const std::string &ContentType, int * length){
    std::string Response = "HTTP/1.1 ";
    Response += responseCode + "\n";



    std::string ResponseHtml = "";
    ResponseHtml += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
    ResponseHtml += "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"pl\" lang=\"pl\">\n\n";

    ResponseHtml += "<head>";
    ResponseHtml += "  <title>" + responseCode + "</title>\n";
    ResponseHtml += "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\" />\n";
    ResponseHtml += "</head>\n";
    ResponseHtml += "<body>\n";
    ResponseHtml += "  " + responseCode + "\n";
    ResponseHtml += "</body>\n";


    ostringstream ss;
    ss << ResponseHtml.length();
    Response += "Content-Length: " + ss.str() + "\n";
    Response += "Content-Type: " + ContentType + "\n\n";
    Response += ResponseHtml;

    *length = Response.length();

    return (char *)Response.c_str();
}

char* genResponseMovedPermanently(const std::string &path, int * length)
{
    std::string Response = "HTTP/1.1 301 Moved Permanently\n";
    Response += "Location: ";
    Response += path;
    Response += '\n';

    *length = Response.length();

    return (char *)Response.c_str();
}


//read all characters from file
char * ReadAllBytes(char const * filename, int * read, bool * fileFound)
{
    ifstream input(filename, std::ifstream::binary);
    if(input)
    {
        *fileFound = true;

        input.seekg (0, input.end);
        int length = input.tellg();
        input.seekg (0, input.beg);

        char * buffer = new char [length];

        input.read (buffer,length);

        input.close();
        *read = length;

        return buffer;
    }
    else
    {
        *fileFound = false;
        *read = -1;

        return NULL;
    }
}

std::string getHost(std::string &hostLine)
{
    myStringArray data = splitString(hostLine, ' ');

    if(data.length < 2) return "";

    std::string temp = data.strArray[1];
    return temp.substr(0, temp.find(':'));
}

std::string getPath(std::string &getLine)
{
    myStringArray data = splitString(getLine, ' ');

    if(data.length < 2) return "";

    return data.strArray[1];
}

std::string type[][2] =
        {   {".txt", "text/plain"},
            {".html", "text/html"},
            {".css", "text/css"},
            {".jpg", "image/jpeg"},
            {".jpeg", "image/jpeg"},
            {".png", "image/png"},
            {".pdf", "application/pdf"} };

std::string getType(std::string &path)
{;
    for(unsigned int i=0; i < sizeof(type)/sizeof(type[0]); ++i)
    {
        if(path.compare(path.length()-type[i][0].length(), type[i][0].length(), type[i][0]) == 0)
        {
            return type[i][1];
        }
    }

    return "application/octet-stream";
}

bool getConnectionClose(std::string &connectionLine)
{
    myStringArray data = splitString(connectionLine, ' ');
    if(data.length < 2) return true;

    if(data.strArray[1].compare("close") == 0) return true;
    else return false;
}

bool checkPath(std::string &path)
{
    myStringArray data = splitString(path, '/');

    int depth = 0;

    if(data.strArray[0].compare("..") == 0) return false;


    for(int i = 0; i<data.length; ++i)
    {
        if(data.strArray[i].compare("") == 0) continue;

        if(data.strArray[i].compare("..") == 0) --depth;
        else ++depth;

        if(depth < 0) return false;
    }

    return true;
}

bool checkCatalog(std::string &path)
{
    if(path[path.length()-1] == '/') return true;

    myStringArray data = splitString(path, '/');

    std::size_t found = data.strArray[data.length-1].find('.');
    if (found==std::string::npos) return true;

    return false;
}

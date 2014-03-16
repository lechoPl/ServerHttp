#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <iostream>
#include "sockwrap.h"
#include "utility.h"

#define BUFFSIZE 10000000	// 10 MB
char buffer[BUFFSIZE+1];

using namespace std;


int main(int argc, char** argv)
{
    if (argc != 3) { cout << "Usage: ./server-http <port> <folder>\n"; exit(1); }

    int port = atoi(argv[1]);
    std::string folder = argv[2];
    if(folder[folder.length()-1] != '/') folder += '/';

	int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in server_address;
	bzero (&server_address, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind (sockfd, &server_address, sizeof(server_address));

	// TCP: rozmiar kolejki na trzymanie zadan polaczenia
	Listen (sockfd, 64);

    cout << "server HTTP" << endl;
    cout << "port: " << port << endl;
    cout << "folder: " << folder << endl;
    cout << "===========" << endl;

    timeval t;

	while (1) {
        sockaddr_in client_address;
		int conn_sockfd = Accept (sockfd, NULL, NULL);

		char ip_address[20];
		inet_ntop (AF_INET, &client_address.sin_addr, ip_address, sizeof(ip_address));
		cout << "# connect: " << ip_address << ":" << ntohs(client_address.sin_port) << endl;

        t.tv_sec = 1; //skundy
        t.tv_usec = 0;//mikrosekundy

        fd_set socks;
        FD_ZERO(&socks);
        FD_SET(conn_sockfd, &socks);

        while (Select(conn_sockfd + 1, &socks, NULL, NULL, &t) > 0)
        {
            int msglen = Recv(conn_sockfd, buffer, BUFFSIZE, 0);
            buffer[msglen] = 0;
            std::string strBuffer = (std::string)buffer;

            myStringArray bufferLines = splitString(strBuffer, '\n');

            std::string importantData[3];
            for(int i=0; i< bufferLines.length; ++i)
            {
                if(bufferLines.strArray[i].compare(0,3, "GET") == 0)
                    importantData[0] = bufferLines.strArray[i];
                if(bufferLines.strArray[i].compare(0,5, "Host:") == 0)
                    importantData[1] = bufferLines.strArray[i];
                if(bufferLines.strArray[i].compare(0,10, "Connection") == 0)
                    importantData[2] = bufferLines.strArray[i];
            }

            //pokazanie danych zadania
            for(int i=0; i<3; ++i)
            {
                cout << '\t' << importantData[i] << endl;
            }
            cout << endl;

            //zmienne do odpowiedzi
            int replLength = 0;
            char* repl;


            //close connection
            bool closeConnection = false;
            if(getConnectionClose(importantData[2]))
            {
                closeConnection =  true;
            }

            //nie zaimplementowane
            if( importantData[0].compare("") == 0 )
            {
                repl = genResponseError( "501 Not Implemented", "text/html", &replLength );
                Send ( conn_sockfd, repl, replLength, 0 );

                if( closeConnection ) break;

                continue;
            }

            std::string host = getHost(importantData[1]);

            std::string path = getPath(importantData[0]);

            //sprawdzenie czy nalezy do domeny
            if(!checkPath(path))
            {
                repl = genResponseError( "403 Forbidden", "text/html", &replLength );
                Send ( conn_sockfd, repl, replLength, 0 );

                if( closeConnection ) break;

                continue;
            }

            //przekrieowanie na index.html
            if(checkCatalog(path))
            {
                if(path[path.length()-1] == '/') path += "index.html";
                else path += "/index.html";

                repl = genResponseMovedPermanently(path, &replLength);

                Send ( conn_sockfd, repl, replLength, 0 );

                if( closeConnection ) break;

                continue;
            }



            std::string type = getType(path);

            const char * filename = ( folder + host + path ).c_str();

            int read = 0;
            bool fileFound = false;
            char * data = ReadAllBytes(filename, &read, &fileFound);

            //brak pliku
            if(!fileFound)
            {
                repl = genResponseError( "404 Not Found", "text/html", &replLength );
                Send ( conn_sockfd, repl, replLength, 0 );

                if( closeConnection ) break;

                continue;
            }



            repl = genResponse( "200 OK", type, read, data, &replLength );


            Send ( conn_sockfd, repl, replLength, 0 );

            if( closeConnection ) break;
        }
        // Konczymy komunikacje z tym gniazdem, wracamy do obslugi gniazda
		// nasluchujacego.
		cout << "# connection close" << endl;
		Close ( conn_sockfd );
	}
}

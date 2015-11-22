/***************************************************************************
 *   ______ _   _      ___   ___ ___         _        _ 
 *  |  ____| | | |    / _ \ / _ \__ \       | |      | |
 *  | |__  | |_| |__ | | | | | | | ) |   ___| |_ _ __| |
 *  |  __| | __| '_ \| | | | | | |/ /   / __| __| '__| |
 *  | |____| |_| | | | |_| | |_| / /_  | (__| |_| |  | |
 *  |______|\__|_| |_|\___/ \___/____|  \___|\__|_|  |_|
 *
 *  v0.1 - 2015-11-22: copyleft(c) 2015 Allard Lamberink
 *
 *  C++ program for switching relays from the command line, using
 *  the ETH002 board from http://www.robot-electronics.co.uk/
 *
 *  Example use case: switching relays from a telephone connected to
 *  an embedded Asterisk Telephony system.
 *
 *  WARNING: use at your own risk, no warranty!!! code might be buggy
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <exception>

using namespace std;
using boost::property_tree::ptree;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}


void readfile(const std::string &filepath,std::string &buffer){
	std::ifstream fin(filepath.c_str());
        getline(fin, buffer, char(-1)); 
        fin.close();
}


string getXmlProp(string xmlstring, string xpath)
{
	string retval = "Error reading XML property";

	//// Read the XML config string into the property tree. Catch any exception
	try {
		ptree pt;
		stringstream xmlstream;
	       	xmlstream << xmlstring;
		read_xml(xmlstream, pt);
		retval = pt.get<std::string>(xpath);
	}
	catch (std::exception &e)
	{
		perror("Error reading xml property: ");
	}
	return retval;
}


int main(int argc, char *argv[])
{
	int sockfd, iPortNo, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char *cmdbuffer = new char[4];
    	bzero(cmdbuffer,sizeof(cmdbuffer));
        const string runningpath = argv[0];
	const string settingsfullpath = runningpath.substr(0,runningpath.find_last_of("/")) + "/settings.xml";
	//	printf("using settingsfile:%s\n",settingsfullpath.c_str());
	if (argc == 3 || argc == 4)
	{
		switch(argv[1][0])
		{       
			case 's':
				if( (argv[3][0] == '1' || argv[3][0] == '0') && (argv[2][0] == '1' || argv[2][0] == '2') )
				{	
					bzero(cmdbuffer,4);
					argv[3][0] == '1' ? cmdbuffer[0]='\x20' : cmdbuffer[0]='\x21'; // 0x20=on, 0x21=off
					argv[2][0] == '1' ? cmdbuffer[1]='\x01' : cmdbuffer[1]='\x02'; // relay nr 0x01 or 0x02
					cmdbuffer[2] = '\x00'; // switch on/off forever
					printf("switch relay %c to state %c\n", argv[2][0], argv[3][0]);
				}
				else
				{
					printf("invalid arguments\n");
					exit(0);
				}
				break;  
			case 't':
				// TODO implement status
				break;
			default:
				exit(0);
				break;
		}
	}
	else
	{
		printf("ETH002-ctrl version 0.1\n");
		printf("No arguments given, usage:\n");
		printf("%s switch 1 1 (switch relay 1 on [1] or off [0])\n", argv[0]);
		printf("%s status 2 (get status of relay 2: returns [1]: on, or [0]: off)\n\n", argv[0]);
		//fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}


	string settingsstr = "";
	/* read settings file */
	try
	{
		readfile(settingsfullpath, settingsstr);
	}
	catch (std::exception &e)
	{
		perror("Error loading settings file, make sure settings.xml exists and is readable. Original errormessage: ");
	}

	/* initialize variables */ 
	if(settingsstr.empty())
	{
		perror("Error loading settings file, make sure settings.xml exists, is readable and is not empty.");
		return 1;
	}

	iPortNo = atoi(getXmlProp(settingsstr, "settings.eth002port").c_str());
	string sIP = getXmlProp(settingsstr, "settings.eth002ip");
	string sPass = "y" + getXmlProp(settingsstr, "settings.eth002pass");

	const char *eth002ip = sIP.c_str();
	const char *eth002pass = sPass.c_str();
   
	fprintf(stdout,"using ip=%s and port=%d\n\n", eth002ip, iPortNo);
    	
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // 0 means auto detect UDP/TCP

	if (sockfd < 0) 
        	error("ERROR opening socket");

    	server = gethostbyname(eth002ip);
	if (server == NULL) {
       		fprintf(stderr,"ERROR, no such host\n");
	        exit(0);
    	}

	/* set all values in buffer to zero */
	bzero((char *) &serv_addr, sizeof(serv_addr));

	/* set the address Family */
	serv_addr.sin_family = AF_INET;

	/* set the ip address */
	bcopy((char *)server->h_addr, 
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);

	/* set the portno (convert to byte-order format) */
	serv_addr.sin_port = htons(iPortNo);

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");


    	n = write(sockfd,eth002pass,strlen(eth002pass));

	if (n < 0) 
    		error("ERROR writing to socket");

	char *result = new char[1];
	bzero(result,sizeof(result));
	/* read password action result */
	n = read(sockfd,result,1);

	if(result[0]==1 || (strlen(eth002pass)==1 && result[0]==0x79)) // 0x79/y means: not set
	{
		printf("password correct\n");
		n = write(sockfd,cmdbuffer,sizeof(cmdbuffer)); // use sizeof here and not strlen because 0x00 will 
							       // be ignored if value is zero
		bzero(result,sizeof(result));
		bzero(cmdbuffer,sizeof(cmdbuffer));
		n = read(sockfd,result,1);
		if(result[0]==0)
		{
			printf("switch ok\n");
		}
		else
		{
			printf("switch failed\n");
		}
	}
	else{
		printf("password incorrect\n");
	}
		
	if (n < 0) 
		error("ERROR reading from socket");

	close(sockfd);
	return 0;
}

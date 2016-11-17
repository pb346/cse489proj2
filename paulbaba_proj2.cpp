/*
 * update (send)
 * define message structure
 * structure for routing table
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>

#define ERROR  (-1)
#define OK     (0)
#define unresponseCount (5)

int distancePackets = 0;


/* Routing Table Data Structure */
struct Link
{
	int ID;
	int cost;
};
struct Node
{
	int serverID;
	int port;
	char IP[16];
	std::vector<Link*> neighbors;
	Node* next;
};

/* Routing Table Data Structure */

/* Message Structure */
struct dataChunk
{
	uint16_t serverPort1;
	uint16_t zeroVal;
	uint16_t serverID1;
	uint16_t cost;
};

struct header
{
	int numFields;
	int serverPort;
	char serverIP[16];
};
/*Message Structure */

void displayHelp()
{
	printf("\n|--------------|\n");
	printf("| 1) help      |\n");
	printf("| 2) display   |\n");
	printf("| 3) update    |\n");
	printf("| 4) step      |\n");
	printf("| 5) packets   |\n");
	printf("| 6) disable   |\n");
	printf("| 7) crash     |\n");
}
void updateNeighbors(Node* ServerList, int hostIndex, int numServers, int type, int optVal)
{
	//first send neighbors updates
	//then expect packets from neighbors
	if(type == 0 || type== 1)
	{
		int fd;
		struct sockaddr_in sockInfo;
		char buf[560];
		Node* iter = ServerList;
		Node* neigh = ServerList;
		Node* temp = ServerList;

		while(iter != NULL) // find what node current server is
		{
			if(iter->serverID == hostIndex)
			{
				break;
			}
			iter = iter->next;
		}

		memset(buf, 0, sizeof(buf));
		int size = iter->neighbors.size();
		uint16_t u16 = htons(type);
		memcpy(buf, &u16, 2);
		u16 = htons(size);
		memcpy(buf+2, &u16, 2);
		u16 = htons(iter->port);
		memcpy(buf+4, &u16, 2);
		inet_pton(AF_INET, iter->IP , &(sockInfo.sin_addr));
		memcpy(buf+6, &iter->IP, 16);
		int offset = 22;

		fd = socket(AF_INET, SOCK_DGRAM, 0);
		struct sockaddr_in sockServer;

		for(int i = 0; i< (int)iter->neighbors.size(); i++)
		{
			neigh = ServerList; //reset
			int counter = 0;
			while(neigh != NULL)
			{
				if(neigh->serverID == iter->neighbors.at(i)->ID)
				{
					dataChunk* chunk = new dataChunk;
					memcpy(buf+offset, &neigh->IP, 16);
					offset += 16;
					chunk->serverPort1 = htons(neigh->port);
					chunk->zeroVal = htons(0x0);
					chunk->serverID1 = htons(neigh->serverID);
					chunk->cost = htons(iter->neighbors.at(i)->cost);
					/* 64 bits */
					memcpy(buf+offset, &chunk->serverPort1, 2);
					memcpy(buf+offset+2, &chunk->zeroVal, 2);
					memcpy(buf+offset+4, &chunk->serverID1, 2);
					memcpy(buf+offset+6, &chunk->cost, 2);
					/* 64 bits */
					offset+=8;
					delete chunk;
				}
				neigh = neigh->next;
			}
		}

		for(int i = 0; i < (int)iter->neighbors.size(); i++)
		{
			neigh = ServerList;
			while(neigh != NULL)
			{
				if(neigh->serverID == iter->neighbors.at(i)->ID && (neigh->serverID != hostIndex))
				{
					if(iter->neighbors.at(i)->cost == -1)
					{
						neigh = neigh->next;
						continue;
					}
					memset((char*)&sockServer, 0, sizeof(sockServer));
					sockServer.sin_family = AF_INET;
					sockServer.sin_port = htons(neigh->port);
					sockServer.sin_addr.s_addr = inet_addr(neigh->IP);
					int addrlen = sizeof(sockServer);
					sendto(fd,buf,sizeof(buf),0, (struct sockaddr *)&sockServer, addrlen);

				}
				neigh = neigh->next;
			}
		}
	}
	/*
	if(type == 1)
	{
		int fd;
		struct sockaddr_in sockServer;
		char buf[560];
		Node* iter = ServerList;
		Node* neigh = ServerList;
		Node* temp = ServerList;

		while(iter != NULL) // find what node current server is
		{
			if(iter->serverID == hostIndex)
			{
				break;
			}
			iter = iter->next;
		}
		memset(buf, 0, sizeof(buf));
		uint16_t u16 = htons(type);
		memcpy(buf, &u16, 2);
		u16 = htons(hostIndex);
		memcpy(buf+2, &u16, 2);
		//send server that is disconnecting
		memset((char*)&sockServer, 0, sizeof(sockServer));
		Node* disableIt = ServerList;
		while(disableIt->serverID != optVal)
			disableIt = disableIt->next;

		sockServer.sin_family = AF_INET;
		sockServer.sin_port = htons(disableIt->port);
		sockServer.sin_addr.s_addr = inet_addr(disableIt->IP);
		int addrlen = sizeof(sockServer);
		sendto(fd,buf,sizeof(buf),0, (struct sockaddr *)&sockServer, addrlen);

	}*/
}

void update(Node** ServerList, int ID1, int ID2, int lcost, int hostIndex)
{
	Node* iter = *ServerList;
	Node* iter2 = *ServerList;
	int existID1Flag = 0;
	int existID2Flag = 0;
	while(iter != NULL) // find what node current server is
	{
		if(iter->serverID == ID1)
		{
			existID1Flag = 1;
			break;
		}
		iter = iter->next;
	}
	while(iter2 != NULL)
	{
		if(iter2->serverID == ID2)
		{
			existID2Flag = 1;
			break;
		}
		iter2 = iter2->next;
	}
	if(existID1Flag == 0)
	{
		printf("update %i, %i ", ID1, ID2);
		if(lcost == -1)
		{
			printf("inf ");
		}
		else
		{
			printf("%i ", lcost);
		}
		printf("ERROR: Server %i is not on file\n", ID2 );
		return;
	}
	if(existID2Flag == 0)
	{
		printf("update %i, %i ", ID1, ID2);
		if(lcost == -1)
		{
			printf("inf ");
		}
		else
		{
			printf("%i ", lcost);
		}
		printf("ERROR: Server %i is not on file\n", ID2 );
		return;
	}
	if(lcost < -1 )
	{
		printf("update %i, %i, %i Error: Not a valid cost\n",ID1, ID2, lcost);
		return;
	}

	existID1Flag = 0;
	existID2Flag = 0;
	for(int i = 1; i < (int)iter->neighbors.size()+1; i++)
	{
		if( iter->neighbors.at(i-1)->ID == ID2)
		{
			existID1Flag = i;
			iter->neighbors.at(i-1)->cost = lcost;
			break;
		}
	}
	if(existID1Flag == 0)
	{
		printf("update %i, %i, %i Error: Server %i not neighboring Server %i\n",ID1, ID2, lcost, ID1, ID2);
		return;
	}
	else
	{
		printf("update %i %i ", ID1, ID2);
		if(lcost == -1)
		{
			printf("inf SUCCESS\n");
		}
		else
			printf("%i SUCCESS\n", lcost);
		iter->neighbors.at(existID1Flag - 1)->cost = lcost;
	}
}

int parseUpdate(char* input, int* ID1, int* ID2, int* lcost, int index)
{
	char buffer[64];
	int count = 0;
	index++;
	while(input[index] != ' ' && input[index] != '\0')
	{
		if(input[index] < 48 || input[index] > 58)
		{
			printf("ERROR: Invalid Argument 1");
			printf("Usage: update <ServerID> <DestinationID> <linkCost>\n");
			return ERROR;
		}
		count ++;
		index++;
	}
	memset(buffer, 0, sizeof buffer);
	memcpy(buffer, input + index - count, count);
	*ID1 = atoi(buffer);
	count = 0;
	index++;
	while(input[index] != ' ' && input[index] != '\0')
	{
		if(input[index] < 48 || input[index] > 58)
		{
			printf("ERROR: Invalid Argument 2");
			printf("Usage: update <ServerID> <DestinationID> <linkCost>\n");
			return ERROR;
		}
		count ++;
		index++;
	}
	memset(buffer, 0, sizeof buffer);
	memcpy(buffer, input + index - count, count);
	*ID2 = atoi(buffer);
	count = 0;
	index++;
	while(input[index] != ' ' && input[index] != '\0')
	{
		count ++;
		index ++;
	}
	memset(buffer, 0, sizeof buffer);
	memcpy(buffer, input + index - count, count);
	if(strcmp(buffer, "inf") == 0)
	{
		*lcost = -1;
	}
	else if(buffer[0] >= '0' && buffer[0] <= '9' )
	{
		*lcost = atoi(buffer);
	}
	else
	{
		printf("ERROR: Invalid Argument 3");
		printf("Usage: update <ServerID> <DestinationID> <linkCost>\n");
		return ERROR;
	}
	return OK;
}

void display(Node* ServerList, int hostIndex)
{
	printf(" _______________________________________________\n");
	printf("| Destination ID       Next-Hop ID        Cost  |\n");
	printf("|_______________________________________________|\n");

	Node* iter = ServerList;
	Node* iter2 = ServerList;
	std::vector<int> neighbors;
	int counter = 1;
	while(iter!= NULL)
	{
		for(int i = 0; i < (int)iter->neighbors.size(); i++)
		{
			printf("        %i                 %i",
					counter, iter->neighbors.at(i)->ID);
			neighbors.push_back(iter->neighbors.at(i)->ID);
			if(iter->neighbors.at(i)->cost == -1 || iter->neighbors.at(i)->cost > 30)
			{
				printf("		inf\n");
			}
			else
			{
				printf("		%i\n", iter->neighbors.at(i)->cost);
			}

		}
		counter+=1;
		iter = iter->next;
	}
	/*
	for(int i = 0; i < (int)neighbors.size(); i++)
	{
		iter2 = ServerList;
		while(iter2 != NULL)
		{
			if(iter2->serverID == neighbors.at(i) && (iter2->serverID != (hostIndex)))
			{
				printf("        %i                 %i",
						iter2->serverID, 0);

				printf("		inf\n");

			}
			iter2 = iter2->next;
		}
	}*/
	printf("\ndisplay SUCCESS\n");

}
Node* readInput(int hostIndex, char* file, char* localIP, int* port, int* numServers)
{
	Node* ServerList;
	Node* current;
	int counter = 0;
	int IPLength = 0;
	int portLength = 0;
	int selfFlag = 0;
	int numNeighbors;
	std::ifstream fileReader(file);
	char line[32];
	char portBuf[5];
	char link[8];
	Node* iter;
	memset(line, 0, 32);
	if(fileReader.fail())
	{
		printf("ERROR: Could not open file\n");
		return NULL;
	}
	while( fileReader.getline (line, 32))
	{
		IPLength = 0;
		portLength = 0;
		if(0 == counter)
			*numServers = atoi(line);
		if(1 == counter)
			numNeighbors = atoi(line);
		if(counter >=2 && counter < *numServers+2)//servers
		{
			if(counter == 2)
			{
				ServerList = new Node;
				current = ServerList;
			}
			if(counter > 2)
			{
				current->next = new Node;
				current = current->next;
			}
			current->next = NULL;
			current->serverID = counter - 1; 						//verify
			char* iter = line;
			iter++;
			iter++;
			while(*iter != ' ')
			{
				IPLength+=1;
				iter++;
			}
			memcpy(current->IP, line+2, IPLength);
			iter++;
			while(*iter != '\0')
			{
				portLength +=1;
				iter++;
			}
			memcpy(portBuf, line+3+IPLength, portLength );
			current->port = atoi(portBuf);
			if(hostIndex == counter - 1)
			{
				memcpy(localIP, current->IP, 64);
				*port = current->port;

			}
		}
		if(counter >= *numServers+2)	//links
		{
			memcpy(link, line, sizeof link);
			iter = ServerList;
			for(int i =0; i < hostIndex -1; i++)
			{
				iter= iter->next;
			}
			if(iter->serverID == hostIndex && selfFlag == 0)
			{
				selfFlag = 1;
				printf("ADDING SELF\n");
				Link* newLink = new Link;
				newLink->ID = hostIndex;
				newLink->cost = 0;
				iter->neighbors.push_back(newLink);
			}
			Link* newLink = new Link;
			newLink->ID = link[2]- 48;
			newLink->cost = link[4] - 48;
			iter->neighbors.push_back(newLink);
		}

		counter+=1;
	}
	fileReader.close();
	return ServerList;
}

int getHostIndex()
{
	char hostname[64];
	gethostname(hostname, 63);
	hostname[63] = '\0';
	//printf("Host: %s\n", hostname);
	if(hostname[0] == 't') //timberlake
		return 1;
	else if(hostname[0] == 'h') //highgate
		return 2;
	else if(hostname[0] == 'u') //underground
		return 3;
	else if(hostname[0] == 'e' && hostname[1] == 'u') //euston
		return 4;
	else if(hostname[0] == 'e' && hostname[1] == 'm') //embankment
		return 5;
	else
		printf("ERROR: Not running on approved server\n");
	return ERROR;
}

void cleanup(Node* ServerList)
{
//	printf("CLEANUP\n");
	Node* iter = ServerList;
	Node* current;
	while(iter->next != NULL)
	{
		current = iter;
		iter = iter->next;
		delete current;
	}
	delete iter;
}

int setupReceive(int port)
{
	struct sockaddr_in  currentSock;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	memset((char*) &currentSock, 0 , sizeof(currentSock));
	currentSock.sin_family = AF_INET;
	currentSock.sin_port = htons(port);
	currentSock.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(fd, (struct sockaddr*)&currentSock, sizeof(currentSock)) < 0)
	{
		perror("ERROR: Current Server Bind");
	}
	return fd;
}
void readMessage(int fd, int* packetCount, Node* ServerList, int hostIndex, int* receivedServers, int* disableList)
{
	char buf[560];
	struct sockaddr_in nothing;
	Node* iter = ServerList;
	Node* iter2 = ServerList;
	/*memset((char*)&receive, 0, sizeof(receive));
	receive.sin_family = AF_INET;
	receive.sin_port = htons(4091);
	receive.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(fd, (struct sockaddr *) &receive, sizeof(receive) )<0)
		perror("BIND");
	*/
	socklen_t addrlen = sizeof(nothing);
	memset(buf, 0, sizeof(buf));
	int recvlen = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&nothing, &addrlen);
	if(recvlen >0)
	{
		//packetCount+=1;
		int val;
		int port;
		char IP[16];
		uint16_t u16;
		uint32_t u32;
		char inIP[16];
		memcpy(&u16, buf, 2);
		int type = ntohs(u16);
		if(type == 0)
		{
			*packetCount+=1;
		}
		/*header*/
		memcpy(&u16, buf+2, 2);
		val = ntohs(u16);
		memcpy(&u16, buf+4, 2);
		port = ntohs(u16);
		memcpy(&IP, buf+6, 16);
		int senderID = -1;
		if(strcmp(IP, "128.205.36.8") == 0)
			senderID = 1;
		else if(strcmp(IP, "128.205.36.33") == 0)
			senderID = 2;
		else if(strcmp(IP, "128.205.36.36") == 0)
			senderID = 3;
		else if(strcmp(IP, "128.205.36.35") == 0)
			senderID = 4;
		else if(strcmp(IP, "128.205.36.34") == 0)
			senderID = 5;
		receivedServers[senderID-1] = 1;
		if(disableList[senderID-1] == 1)
		{
			return;
		}
		if(type == 1)
		{
			printf("BEING DISABLED\n");
			printf("From sender %i\n", senderID);
			Node* iter = ServerList;
			while(iter->serverID != hostIndex)
				iter = iter->next;
			Node* iter2 = ServerList;
			while(iter2->serverID != senderID)
				iter2 = iter2->next;

			for(int i = 0; i < iter->neighbors.size(); i++)
			{
				if(iter->neighbors.at(i)->ID == senderID)
				{
					iter->neighbors.at(i)->cost = -1;
					break;
				}
			}
			for(int i = 0; i < iter2->neighbors.size(); i++)
			{
				if(iter2->neighbors.at(i)->ID == hostIndex)
				{
					iter2->neighbors.at(i)->cost = -1;
					break;
				}
			}
			return;

		}

		printf("RECEIVED A MESSAGE FROM SERVER %i\n", senderID);
		while(iter->serverID != (senderID))
			iter = iter->next;
		int offset = 22;
		int index  = 0;
		for( int i = 0; i < val; i++)
		{
			memcpy(&inIP, buf+offset, 16);
			offset+= 16;
			memcpy(&u16, buf+offset, 2);
			int port1 = ntohs(u16);
			memcpy(&u16, buf+offset+2, 2);
			int zero = ntohs(u16);
			memcpy(&u16, buf+offset+4, 2);
			int ID = ntohs(u16);
			memcpy(&u16, buf+offset+6, 2);
			int cost = ntohs(u16);
			offset += 8;
			/*
			if(strcmp(inIP, "128.205.36.8"))
				index = 1;
			if(strcmp(inIP, "128.205.36.33"))
				index = 2;
			if(strcmp(inIP, "128.205.36.36"))
				index = 3;
			if(strcmp(inIP, "128.205.36.35"))
				index = 4;
			if(strcmp(inIP, "128.205.36.34"))
				index = 5;
			*/
			iter2 = ServerList;
			int dupFlag = 0;
			for(int i = 0; i < iter->neighbors.size(); i++)
			{
				if(iter->neighbors.at(i)->ID == ID)
				{
					if(iter->neighbors.at(i)->cost == cost)
					{
						dupFlag = 1;
					}
					else
					{
						iter->neighbors.at(i)->cost = cost;
						dupFlag = 1;
					}
				}

			}
			if(dupFlag == 0)
			{
				Link* newLink = new Link;
				newLink->ID = ID;
				newLink->cost = cost;
				iter->neighbors.push_back(newLink);

			}
		}
		return;
	}
}

int disable(Node** ServerList, int disableID, int hostIndex)
{
	Node* iter = *ServerList;
	Node* iter2 = *ServerList;
	int found = 0;
	while(iter->serverID != hostIndex)
		iter = iter->next;
	for(int i = 0; i < iter->neighbors.size(); i++)
	{
		if(iter->neighbors.at(i)->ID == disableID)
		{
			found+=1;
		}
	}
	while(iter2->serverID != disableID)
		iter2 = iter2->next;

	for(int i = 0; i < iter2->neighbors.size(); i++)
	{
		if(iter2->neighbors.at(i)->ID == hostIndex)
		{
			printf("Dest %i, NextHop %i\n", iter2->serverID, hostIndex);
			iter2->neighbors.at(i)->cost = -1;
			found+=1;
		}
	}
	if(found != 2)
	{
		printf("update %1$i Error: Server %1$i is not a neighbor\n", disableID);
		return 0;
	}
	return 1;

}
int main(int argc, char* argv[])
{
	printf("Starting Program\n");
	int disableList[5];
	char file[128];
	int packetCount = 0;
	char localIP[64];
	char command[16];
	int receivedServers[5];
	int missedServers[5];
	int localPort;
	int setupDelay = 0;
	int interval;
	int sendStatus = 0;
	int receiveCount = 0;
	int numServers;
	if(argc < 4)
	{
		printf("ERROR: Incorrect arguments\n");
		printf("Usage: ./server -t <topology-file-name> -i <routing-update-interval>\n");
		return ERROR;
	}
	else
	{
		if(strcmp(argv[1], "-t") != 0 && strcmp(argv[3], "-t") != 0)
		{
			printf("ERROR: Incorrect flag argument 1\n");
			return ERROR;
		}
		if(strcmp(argv[3], "-i") != 0 && strcmp(argv[1], "-i") != 0)
		{
			printf("ERROR: Incorrect flag argument 3\n");
			return ERROR;
		}
		if(strcmp(argv[1], "-t") == 0)
		{
			memcpy(file, argv[2], 128);
			interval = atoi(argv[4]);
		}
		else
		{
			memcpy(file, argv[4], 128);
			interval = atoi(argv[2]);
		}
	}
	std::vector<int> temp;
	Node* ServerList;
	temp.push_back(0);
	int hostIndex = getHostIndex();
	missedServers[hostIndex-1] = -1;
	receivedServers[hostIndex-1] = -1;
	ServerList = readInput(hostIndex, file, localIP, &localPort, &numServers);
	int Masterfd = setupReceive(localPort);
	if(ServerList == NULL)
	{
		return -1;
	}
	printf("\nIP: %s\n", localIP);
	printf("Port %i\n\n", localPort);
	fd_set fdList;
	struct timeval tv;
	char input[32];
	while(true)
	{
		tv.tv_sec = interval;
		tv.tv_usec = 0;
		FD_ZERO(&fdList);
		FD_SET(0, &fdList);
		FD_SET(Masterfd, &fdList);
		int selectVal = select(Masterfd+1, &fdList, NULL, NULL, &tv);

		if( selectVal == -1)
		{
			printf("ERROR: select()\n");
		}

		if(FD_ISSET(Masterfd, &fdList))
		{
			if(receiveCount == 2)
			{
				updateNeighbors(ServerList, hostIndex, numServers, 0, 0);
				receiveCount = 0;
				continue;
			}
			readMessage(Masterfd, &packetCount, ServerList, hostIndex, receivedServers, disableList);
			receiveCount += 1;
		}

		if(FD_ISSET(0, &fdList)) //process user input
		{
			printf("Processing Input\n");
			memset(input, 0, 32);
			memset(command, 0, 16);
			int index = 0;
			std::cin.getline(input, 32);
			while(input[index] != ' ' && input[index] != '\0')
				index++;
			memcpy(command, input, index);
			if(command[0] == 'q' || strcmp(command, "quit") == 0)
				break;
			if(strcmp(command, "display") == 0)
				display(ServerList, hostIndex);
			if(strcmp(command, "update") == 0)
			{
				//printf("Updating\n");
				int ID1, ID2, lcost;
				if(parseUpdate(input, &ID1, &ID2, &lcost, index) == ERROR)
					continue;
				update(&ServerList, ID1, ID2, lcost, hostIndex);
				printf("%i %i %i\n", ID1, ID2, lcost);
			}
			if(strcmp(command, "step") == 0)
			{
				updateNeighbors(ServerList, hostIndex, numServers, 0, 0);
				printf("Step Successful\n");
			}
			if(strcmp(command, "packets") == 0)
			{
				printf("Packet Count: %i\n", packetCount);
				packetCount = 0;
				printf("packet SUCCESS\n");
			}
			if(strcmp(command, "crash") == 0)
			{
				printf("Crash\n");
			}
			if(strcmp(command, "disable") == 0)
			{
				printf("Disable\n");
				char IDbuf[8];
				index++;
				memcpy(IDbuf, input + index, 8);
				int disableID = atoi(IDbuf);
				if(disable(&ServerList, disableID, hostIndex))
				{
					disableList[disableID-1] = 1;
					updateNeighbors(ServerList, hostIndex, numServers, 1, disableID);
					update(&ServerList, hostIndex, disableID, -1, hostIndex);
					printf("disable %i SUCCESS\n", disableID);
				}


			}
			if(strcmp(command, "help") == 0)
			{
				displayHelp();
			}

		}
		if( selectVal == 0) //timeout
		{
			printf("SENDING\n");
			sendStatus += 1;
			updateNeighbors(ServerList, hostIndex, numServers, 0, 0);
			/*
			if(setupDelay != 0)
			{
				for(int i = 0; i < 5; i++)
				{
					if(receivedServers[i] == 1)
					{
						missedServers[i] = 0;
					}
					else if(receivedServers[i] == -1)
					{
						continue;
					}
					else
					{
						missedServers[i] += 1;
						printf("MISS %i\n", i+1);
					}
					receivedServers[i] = 0; //reset
				}
			}*/

			Node* iter = ServerList;
			bool isNeighbor = false;
			while(iter->serverID != hostIndex)
				iter = iter->next;
			for(int i = 0; i < 5; i++)//check all servers
			{
				/*
				printf("CHECKING %i\n", i+1);
				isNeighbor = false;
				for(int j = 0; j <iter->neighbors.size(); j++)//check current server neighbors
				{
					printf("LOOKING LOCAL %i\n", iter->neighbors.at(j)->ID);
					if(iter->neighbors.at(j)->ID == (i+1))
					{
						isNeighbor = true;
						break;
					}
				}
				*/
				if((missedServers[i] == unresponseCount) )//&& isNeighbor)
				{
					printf("SERVER %i SET TO INF\n", i+1);
					update(&ServerList, hostIndex, i+1, -1, hostIndex);
					update(&ServerList, i+1, hostIndex, -1, i+1);
					updateNeighbors(ServerList, hostIndex, numServers, 0, 0);

				}
			}
			setupDelay += 1;	//needs to be a better way
		}
	} // end while


	cleanup(ServerList);
	return 0;
}

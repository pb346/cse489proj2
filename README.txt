Project 2 Documentation

Data Structures
	Routing Table:
		File: paulbaba_proj2.cpp
		Line: 25
	UpdateMessage:
		File: paulbaba_proj2.cpp
		Line: 42

Implementation:
	Routing Table:
		When the program is run, 5 Nodes are created and connected as a singly linked
	list. The Node contains the information for it’s ID, port number, IP and a vector of neighbors.
	All Nodes gather their information for ID, port and IP via the Topology file. The neighbors data 	
	field is only populated on the server that is running the program. All other neighbors data fields
	are empty at the start of the code. The neighbors vector is of type Link. Each Link node consists 
	of the ID of a neighbor and the cost of that link. When updates are sent, the Nodes that are 
	neighbors to the current server will have their neighbors data field filled. These neighbors 
	represent next- hop destinations for the current server. If a server is not directly accessible from
	the server running the code, the link cost will be displayed as inf, and the neighbors data field in
	the inaccessible server node will not be populated. The only thing the server running the 	
	program will know about it is it’s IP, port, and ID numbers.

	Message Structure Distance Vector Updates:
		This code got a little messy because we were given limitations after the project 	
	description was released, and I did not have time to rewrite the entire project so I had to
	make use of the current implementation and work off of that. Originally, I had planed to use 
	a header to indicate the type of message. This would have consisted of the types for an “Update 	
	Message”, “Disable Message”, “Crash Message”. Since this was later on forbidden, the header 
	structure was not used. The dataChunk struct holds all the data for each update field. Line 92
	shows the data for number of update fields, server port, and Server IP being placed into the 	
	message. From there, update fields for each of the servers are placed into the message. Then
	the program iterates through all of the ID’s in the current server’s neighbors data field and
	sends to only those servers.
		
	Functions:
		void displayHelp():
			A useful dialog to show what functions are possible to call when running
		
		void updateNeighbors(Node*, int, int, int, int, int[]):
			This is the function that is called when sending distance vectors. When
		the select function times out, this function is called. It is also used when calling 
		the step command. This function first determines what server the program is on
		by traversing the ServerList linked list. All nodes that have neighbors are added
		to the message structure. Then the program iterates through all of the current server’s
		neighbors and sends this message to them only. If the server knows that the cost of
		a server is inf, either because it was set that way, or because it was disable, it will
		not send the update to them.

		
		void update(Node**, int, int, int, int, int):
			Each parameter is checked to see if they are valid. The command must list
		the current server first. The cost change affects the ServerList linked list directly.
		Both the link to the second parameter and from the second parameter to the current
		server is adjusted. 
		
		int parseUpdate(int, int*, int*, int*, int*, int):
			This function is used to quickly take apart an update message, the values 
		for the ID’s affected and link cost are saved and used to then call update.
		
		void display(Node*, int, int[]):
			It wasn’t until the end of the project that I understood how the displayed				
		command should be outputted so it is a little messy. It is here that the shortest distance
		is calculated and then displayed. The program looks for the shortest path as a direct 			
		neighbor of the current server. If one can not be found, then the program looks through
		the neighbors of each of its own neighbors, comparing each to find the smallest overall
		link cost to a destination server.
		
		Node* readInput(int, char*, char*, int*, int*):
			This function reads the topology.txt file and creates the ServerList linked list.
		The node to the head of the list is returned.
		
		int getHostIndex():
			This function is used to determine the server number of the current server. It is
		done by looking at the hostname and assigning it a ID from that.
		
		void cleanup(Node* ServerList):
			Deletes data from the ServerList to prevent memory leaks. 
		
		int setupReceive(int):
			The server creates a socket and binds it to its receiving port
		
		void readMessage(int, int*, Node**, int, int*, int*, int*):
			This function is the opposite of updateNeighbors. It deconstruction each message
		such that it looks very similar to the updateNeighbors function copy data into the 
		message data structure. Changes in link changes also happens here like the parseUpdate 			
		function. It is here that a server has it’s neighbors’ neighbors data field populated with
		the ID’s and costs of links.
		
		int main(int, char*): 
			The driver of the code. It selects between receiving messages, receiving user 			
		input and timeout after the user defined interval. Lack of updates from neighbors
		are kept track here, and once 3 missing updates happen, the missing server’s link cost
		is set to inf.

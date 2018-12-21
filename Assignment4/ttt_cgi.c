#include <stdio.h>	//printf
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
	char *data;
	char *addr;
	char *port_string;
	unsigned short int port;
	char *username;
	char *password;
	char *gamename;
	char *square;
	char *command_name;
	char *command;

	printf("%s%c%c\n", "Content-Type:text/html;charset=iso-8859-1",13,10);
	printf("<TITLE>Tic Tac Toe Game</TITLE>\n");

	data = getenv("QUERY_STRING");

	if(data == NULL)
		  printf("<p>Error! Error in passing data from form to script. Try again.");


	//Parsing the query string (this is currently very inefficient and needs improvement)	
	char *garbage1 = strtok(data, "=");
	addr = strtok(NULL, "&");
	if (strcmp(addr, "port=") != 0) {
		char *garbage2 = strtok(NULL, "=");
	}
	else {
		addr = "\0";
		printf("<p> Error. An address must be entered. </p>");
	}

	port = (unsigned short int)atoi(strtok(NULL, "&"));
	if (port != 0) {
		char *garbage3 = strtok(NULL, "=");
	}
	else {
		printf("<p> Error. A valid port number must be entered. </p>");
	}

	username = strtok(NULL, "&");
	if (strcmp(username, "password=") != 0) {
		char *garbage4 = strtok(NULL, "=");
	}
	else {
		username = "\0";
	}

	password = strtok(NULL, "&");
	if (strcmp(password, "gamename=") != 0) {
		char *garbage5 = strtok(NULL, "=");
	}
	else {
		password = "\0";
	}

	gamename = strtok(NULL, "&");

	if (strcmp(gamename, "square=") != 0) {
		char *garbage6 = strtok(NULL, "=");
	}
	else {
		gamename = "\0";
	}

	square = strtok(NULL, "&");
	if (strcmp(square, "LOGIN=LOGIN") != 0 && strcmp(square, "CREATE=CREATE") != 0 &&
		strcmp(square, "JOIN=JOIN") != 0 && strcmp(square, "MOVE=MOVE") != 0 &&
		strcmp(square, "LIST=LIST") != 0 && strcmp(square, "SHOW=SHOW") != 0 && strcmp(square, "(null)") != 0) {
		char *garbage7 = strtok(NULL, "=");
		command = strtok(NULL, "&");
	}
	else {
		char *garbage8 = strtok(square, "=");
		command = strtok(NULL, "=");
		square = "\0";
	}
		
	//This is the start of the networking code
	int sock;
	struct sockaddr_in server;
	char message[200], prev_msg[200];

	memset(prev_msg, '\0', 200 );
	
	server.sin_addr.s_addr = inet_addr(addr);
	server.sin_family = AF_INET;
	server.sin_port = htons( port );		
		
	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
		return -1;
	}
	
	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		printf("<p> Error. Failed to connect to server. </p>");
		return 0;
	}

	
	sprintf(message, "%s,%s,%s,%s,%s", command,username,password,gamename,square);

	//Send some data
	if( send(sock , message , 200, 0) < 0)
	{
		puts("Send failed");
		return 1;
	}
	
	size_t read_size;
	int bytes_read = 0;
	char incoming_msg[2000];
	char temp[2000];
	while( bytes_read < 2000 ){
		read_size = recv(sock , temp, 2000, 0);
		if(read_size <= 0){
		    puts("Server disconnected");
		    fflush(stdout);
		    close(sock);
		    return 0;
		}
		memcpy( incoming_msg+bytes_read, temp, read_size );
		bytes_read += read_size;
	}


	char *msg_reader = incoming_msg;
	int counter = 0;

	while (*msg_reader) {
		if (*msg_reader == '\n' || *msg_reader == '-' || *msg_reader == ' ')
			counter++;
		msg_reader++;
	}

	msg_reader = incoming_msg;

	char final_msg[2000+counter*7];
	counter = 0;

	//To ensure that dashes, newlines, and spaces are interpreted correctly in HTML
	while (*msg_reader) {
		if (*msg_reader != '\n' && *msg_reader != '-' && *msg_reader != ' ') { 
			final_msg[counter] = *msg_reader;
			counter++;
		}
		else if (*msg_reader == '\n') {
			final_msg[counter] = '<';
			final_msg[counter+1] = 'b';
			final_msg[counter+2] = 'r';
			final_msg[counter+3] = '>';
			counter += 4;
		}
		else if (*msg_reader == '-') {
			final_msg[counter] = '&';
			final_msg[counter+1] = 'n';
			final_msg[counter+2] = 'd';
			final_msg[counter+3] = 'a';
			final_msg[counter+4] = 's';
			final_msg[counter+5] = 'h';
			final_msg[counter+6] = ';';
			counter+= 7;
		}
		else if (*msg_reader == ' ') {
			final_msg[counter] = '&';
			final_msg[counter+1] = 'e';
			final_msg[counter+2] = 'n';
			final_msg[counter+3] = 's';
			final_msg[counter+4] = 'p';
			final_msg[counter+5] = ';';
			counter+=6;
		}
		msg_reader++;
	}

	final_msg[counter] = '\0';

	//Printing the HTML form and pre-filling all fields that the user filled in on the last entry

	printf("<form action=\"ttt.cgi\">\n<b>Server Address: <input type=\"text\" name=\"address\" value=\"%s\" size=\"20\"><br />\n<b>Server Port: <input type=\"text\" name=\"port\" value=\"%d\" size=\"20\"><br />\n<b>Username: <input type=\"text\" name=\"username\" value=\"%s\" size=\"20\"><br />\n<b>Password: <input type=\"text\" name=\"password\" value=\"%s\" size=\"20\"><br />\n<b>Gamename: <input type=\"text\" name=\"gamename\" value=\"%s\" size=\"20\"><br />\n<b>Square: <input type=\"text\" name=\"square\" value=\"%s\" size=\"20\"><br />\n<input type=\"submit\" value=\"LOGIN\" name=\"LOGIN\">\n<input type=\"submit\" value=\"CREATE\" name=\"CREATE\">\n<input type=\"submit\" value=\"JOIN\" name=\"JOIN\">\n<input type=\"submit\" value=\"MOVE\" name=\"MOVE\">\n<input type=\"submit\" value=\"LIST\" name=\"LIST\">\n<input type=\"submit\" value=\"SHOW\" name=\"SHOW\">\n</form>\n",addr,port,username,password,gamename,square);
	

	printf( "<p> %s </p>\n", final_msg );
	close(sock);
		



	return 0;
}

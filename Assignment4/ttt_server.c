/*********************************************************
* FILE: ttt_server.c
* 
* A starter code for your answer to A4 Q1. Most of the
* networking is already done for you, and we even gave
* suggested data types (structs and linked lists of these)
* to represent users and tic-tac-toe games. You must just
* figure out how to fill in the various functions required
* to make the games happen.
*
* Good luck, and rember to ask questions quickly if you get 
* stuck. My Courses Dicussions is the first place to try, 
* then office hours.
*
* AUTHOR: YOU!
* DATE: Before Dec 3rd
***********************************************************/

#include <stdio.h>
#include <string.h>	
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>	
#include <stdlib.h>
#include <time.h>

struct USER{ //Linked list of users
	char username[100];
	char password[100];
	struct USER *next;
};

enum GAME_STATE{ 
	CREATOR_WON=-2,
	IN_PROGRESS_CREATOR_NEXT=-1,
	DRAW=0,
	IN_PROGRESS_CHALLENGER_NEXT=1,
	CHALLENGER_WON=2
};

struct GAME{ //Linked list of games
	char gamename[100];
	struct USER *creator;
	struct USER *challenger;
	enum GAME_STATE state;
	char ttt[3][3];
	struct GAME *next;
};

struct USER *user_list_head = NULL;
struct USER *user_iterator = NULL;

struct GAME *game_list_head = NULL;
struct GAME *game_list_tail = NULL;
struct GAME *game_iterator = NULL;

int login_status = 0;
int game_status = 0;

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2000];

	unsigned short int port = 8888;

	if( argc > 1 )
		port = (unsigned short int)atoi(argv[1]);
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );
	
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("bind failed. Error");
		return 1;
	}

	listen(socket_desc , 3);

	printf( "Game server ready on port %d.\n", port );

	while( 1 ){
		c = sizeof(struct sockaddr_in);

		//accept connection from an incoming client
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0)
		{
			perror("accept failed");
			return 1;
		}

		char temp[200];
		memset(client_message, '\0', 200);
		int bytes_read = 0;
		while( bytes_read < 200 ){
			read_size = recv(client_sock , temp , 200, 0);
			if(read_size <= 0){
				puts("Client disconnected");
				fflush(stdout);
				close(client_sock);
				close(socket_desc);
				return 0;
			}
			memcpy( client_message+bytes_read, temp, read_size );
			bytes_read += read_size;
		}

	  	char response[2000];
	  	response[0] = '\0';
	  	char* command = strtok( client_message, "," );
	  	char *username = strtok( NULL, "," );
	  	char *password = strtok( NULL, ",");
		struct USER *current_user = NULL;
		struct GAME *current_game = NULL;
		int row = 0;
		int col = 0;

		if( command == NULL || username == NULL || password == NULL ){
			sprintf( response, "MUST ENTER A VALID COMMAND WITH ARGUMENTS FROM THE LIST:\n" );
			sprintf( response+strlen(response), "LOGIN,USER,PASS\n" );
			sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME\n" );
			sprintf( response+strlen(response), "JOIN,USER,PASS,GAMENAME,SQUARE\n" );
			sprintf( response+strlen(response), "MOVE,USER,PASS,GAMENAME,SQUARE\n" );
			sprintf( response+strlen(response), "LIST,USER,PASS\n" );
			sprintf( response+strlen(response), "SHOW,USER,PASS,GAMENAME\n" );
			write(client_sock , response , 2000);  
		  	close(client_sock);
			continue;
		}
		else {
			user_iterator = user_list_head;
			login_status = 0;
			game_status = 0;

			while (user_iterator != NULL) {
				if (strcmp(username, user_iterator->username) == 0) {
					if (strcmp(password, user_iterator->password) == 0) {
						login_status = 1;
						current_user = user_iterator;
					}
					else {
						login_status = 2;
						strcpy(response, "BAD PASSWORD");
					}
				}	
				if (user_iterator->next == NULL) {
					break;
				}
				else {
					user_iterator = user_iterator->next;
				}
			}
		}

		if( strcmp( command, "LOGIN" ) == 0 ){
			if (login_status == 1) 
				strcpy(response, "EXISTING USER LOGIN OK");
			else if (login_status == 0) { //Add to the linked list of users
				struct USER *user = (struct USER*)malloc(sizeof (struct USER));
				strcpy(user->username, username);
				strcpy(user->password, password);
				user->next = NULL;

				if (user_list_head == NULL) {
					user_list_head = user;	
				}
				else {
					user_iterator->next = user;
				}
				strcpy(response, "NEW USER CREATED OK");
			}
				

	  	}
	  	else if( strcmp( command, "CREATE" ) == 0 ){ 
			if (login_status == 0) 
				strcpy(response, "USER NOT FOUND");
			else if (login_status == 1) {
				char *game_name = strtok( NULL, ",");

				if( game_name == NULL ){
					sprintf( response, "CREATE COMMAND MUST BE CALLED AS FOLLOWS:\n" );
					sprintf( response+strlen(response), "CREATE,USER,PASS,GAMENAME\n" );
					write(client_sock , response , 2000);  
					close(client_sock);
					continue;
				}

				game_iterator = game_list_head;

				while (game_iterator != NULL) {
					if (strcmp(game_name, game_iterator->gamename) == 0) {
						sprintf(response, "GAME NAME %s ALREADY EXISTED\n", game_name);
						write(client_sock, response, 2000);
						close(client_sock);
						continue;
					}	
					game_iterator = game_iterator->next;
				}

				struct GAME *game = (struct GAME*)malloc( sizeof(struct GAME) );
				strcpy(game->gamename, game_name);
				game->creator = current_user;
				game->state = 1;

				if (game_list_head == NULL) {
					game_list_head = game;
					game_list_tail = game;
				}
				else {
					game_list_tail->next = game;
					game_list_tail = game;
				}

				for( int row=0; row<3; row++ )
					for( int col=0; col<3; col++ )
						game->ttt[row][col] = ' ';
				
				sprintf( response, "GAME %s CREATED. WAITING FOR OPPONENT TO JOIN.\r\n", game_name);
				sprintf( response, "%sa  %c | %c | %c \r\n",response,  game->ttt[0][0],  game->ttt[0][1],  game->ttt[0][2]);
				sprintf( response, "%s  ---|---|---\r\n", response );
				sprintf( response, "%sb  %c | %c | %c \r\n", response, game->ttt[1][0],  game->ttt[1][1],  game->ttt[1][2]);
				sprintf( response, "%s  ---|---|---\r\n", response );
				sprintf( response, "%sc  %c | %c | %c \r\n", response, game->ttt[2][0],  game->ttt[2][1],  game->ttt[2][2]);
				sprintf( response, "%s\r\n", response );
				sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );
			}
		}
		else if( strcmp( command, "JOIN" ) == 0 ){ //Joining a game as the challenger
			if (login_status == 0) 
				strcpy(response, "USER NOT FOUND");
			else if (login_status == 1) {
				char *game_name = strtok( NULL, ",");
				char *square = strtok(NULL, ",");

				if( game_name == NULL || square == NULL){
					sprintf( response, "JOIN COMMAND MUST BE CALLED AS FOLLOWS:\n" );
					sprintf( response+strlen(response), "JOIN,USER,PASS,GAMENAME,SQUARE\n" );
					write(client_sock , response , 2000);  
					close(client_sock);
					continue;
				}
				
				if (game_list_head == NULL) 
					game_status = 0;
				else {
					game_iterator = game_list_head;
					while (game_iterator != NULL) {
						if (strcmp(game_name, game_iterator->gamename) == 0) {
							game_status = 1;
							current_game = game_iterator;
							break;
						}
						game_iterator = game_iterator->next;
					}
				}

				if (game_status == 0) {
					sprintf(response, "GAME %s DOES NOT EXIST", game_name);
				}
				else if (current_game->challenger != NULL) {
					sprintf(response, "GAME %s ALREADY HAS A CHALLENGER", game_name);
					game_status = 0;
				}
				else if (*square < 'a' || *square > 'c') {
					sprintf(response, "INVALID MOVE %s. ROW MUST BE a-c", square);
					game_status = 0;
				}
				else if (*(square+1) < '1' || *(square+1) > '3') {
					sprintf(response, "INVALID MOVE %s. COL MUST BE 1-3", square);
					game_status = 0;
				}

				if (game_status == 0) {
					write(client_sock,response,2000);
					close(client_sock);
					continue;
				}

				current_game->challenger = current_user;

				sprintf(response, "GAME %s BETWEEN %s AND %s\r\n", game_name, current_game->creator->username, current_game->challenger->username);
				sprintf(response, "%sIN PROGRESS: %s TO MOVE NEXT AS o\r\n", response, current_game->creator->username);

				current_game->state = IN_PROGRESS_CREATOR_NEXT;

				if (*square == 'a') 
					row = 0;
				else if (*square == 'b')
					row = 1;
				else 
					row = 2;
				
				if (*(square+1) == '1') 
					col = 0;
				else if (*(square+1) == '2') 
					col = 1;
				else 
					col = 2;			

				current_game->ttt[row][col] = 'x';	

				sprintf( response, "%sa  %c | %c | %c \r\n",response,  current_game->ttt[0][0],  current_game->ttt[0][1],  current_game->ttt[0][2]);
				sprintf( response, "%s  ---|---|---\r\n", response );
				sprintf( response, "%sb  %c | %c | %c \r\n", response, current_game->ttt[1][0],  current_game->ttt[1][1],  current_game->ttt[1][2]);
				sprintf( response, "%s  ---|---|---\r\n", response );
				sprintf( response, "%sc  %c | %c | %c \r\n", response, current_game->ttt[2][0],  current_game->ttt[2][1],  current_game->ttt[2][2]);
				sprintf( response, "%s\r\n", response );
				sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );

			}

		}
		else if( strcmp( command, "MOVE" ) == 0 ){ //Making a move (for the user whose turn it is)
			if (login_status == 0) 
				strcpy(response, "USER NOT FOUND");
			else if (login_status == 1) {
				char *game_name = strtok( NULL, ",");
				char *square = strtok(NULL, ",");
				
				if( game_name == NULL ){
					sprintf( response, "MOVE COMMAND MUST BE CALLED AS FOLLOWS:\n" );
					sprintf( response+strlen(response), "MOVE,USER,PASS,GAMENAME,SQUARE\n" );
					write(client_sock , response , 2000);  
					close(client_sock);
					continue;
				}

				if (game_list_head == NULL) 
					game_status = 0;
				else {
					game_iterator = game_list_head;
					while (game_iterator != NULL) {
						if (strcmp(game_name, game_iterator->gamename) == 0) {
							game_status = 1;
							current_game = game_iterator;
							break;
						}
						game_iterator = game_iterator->next;
					}
				}

				if (game_status == 0) {
					sprintf(response, "GAME %s DOES NOT EXIST", game_name);
				}
				else if (current_game->state == -2 || current_game->state == 0 || current_game->state == 2) {
					sprintf(response, "CANNOT MAKE A MOVE IN COMPLETED GAME %s\r\n", game_name);
					game_status = 0;
				}
				else if (current_game->challenger == NULL) {
					sprintf(response, "GAME %s DOES NOT HAVE A CHALLENGER\r\n", game_name);
					game_status = 0;
				}
				else if (current_game->state == -1 && strcmp(username, current_game->creator->username) != 0) {
					sprintf(response, "INVALID USER. ONLY %s CAN MAKE THE NEXT MOVE AS x IN GAME %s", current_game->creator->username, game_name);
					game_status = 0;
				}
				else if (current_game->state == 1 && strcmp(username, current_game->challenger->username) != 0) {
					sprintf(response, "INVALID USER. ONLY %s CAN MAKE THE NEXT MOVE AS o IN GAME %s", current_game->challenger->username, game_name);
					game_status = 0;
				}
				else if (*square < 'a' || *square > 'c') {
					sprintf(response, "INVALID MOVE %s. ROW MUST BE a-c", square);
					game_status = 0;
				}
				else if (*(square+1) < '1' || *(square+1) > '3') {
					sprintf(response, "INVALID MOVE %s. COL MUST BE 1-3", square);
					game_status = 0;
				}
				else {

					if (*square == 'a') 
						row = 0;
					else if (*square == 'b')
						row = 1;
					else 
						row = 2;
					
					if (*(square+1) == '1') 
						col = 0;
					else if (*(square+1) == '2') 
						col = 1;
					else 
						col = 2;

					if (current_game->ttt[row][col] != ' ') {
						sprintf(response, "INVALID MOVE %s. SQUARE NOT EMPTY", square);
						game_status = 0;
					}
				}

				if (game_status == 0) {
					write(client_sock,response,2000);
					close(client_sock);
					continue;
				}

			
				sprintf(response, "GAME %s BETWEEN %s AND %s\r\n", game_name, current_game->creator->username, current_game->challenger->username);

				if (current_game->state == -1) { 
					current_game->ttt[row][col] = 'o';
					if ((current_game->ttt[0][0] == 'o' && current_game->ttt[0][1] == 'o' && current_game->ttt[0][2] == 'o') ||
					    (current_game->ttt[1][0] == 'o' && current_game->ttt[1][1] == 'o' && current_game->ttt[1][2] == 'o') ||
					    (current_game->ttt[2][0] == 'o' && current_game->ttt[2][1] == 'o' && current_game->ttt[2][2] == 'o') ||
					    (current_game->ttt[0][0] == 'o' && current_game->ttt[1][0] == 'o' && current_game->ttt[2][0] == 'o') ||
					    (current_game->ttt[0][1] == 'o' && current_game->ttt[1][1] == 'o' && current_game->ttt[2][1] == 'o') ||
					    (current_game->ttt[0][2] == 'o' && current_game->ttt[1][2] == 'o' && current_game->ttt[2][2] == 'o') ||
					    (current_game->ttt[0][0] == 'o' && current_game->ttt[1][1] == 'o' && current_game->ttt[2][2] == 'o') ||
					    (current_game->ttt[0][2] == 'o' && current_game->ttt[1][1] == 'o' && current_game->ttt[2][0] == 'o')) {
						current_game->state = -2;
						sprintf(response, "GAME OVER: %s WON\r\n", current_game->creator->username);
					}
					else if (current_game->ttt[0][0] != ' ' && current_game->ttt[0][1] != ' ' && current_game->ttt[0][2] != ' ' &&
						 current_game->ttt[1][0] != ' ' && current_game->ttt[1][1] != ' ' && current_game->ttt[1][2] != ' ' &&
						 current_game->ttt[2][0] != ' ' && current_game->ttt[2][1] != ' ' && current_game->ttt[2][2] != ' ') {
						current_game->state = 0;
						sprintf(response, "GAME OVER: DRAW\r\n");
					}
					else {
						current_game->state = 1;
						sprintf(response, "%sIN PROGRESS: %s TO MOVE NEXT AS x\r\n", response, current_game->challenger->username);
					}
				}
				else if (current_game->state == 1) {
					current_game->ttt[row][col] = 'x';		
					if ((current_game->ttt[0][0] == 'x' && current_game->ttt[0][1] == 'x' && current_game->ttt[0][2] == 'x') ||
					    (current_game->ttt[1][0] == 'x' && current_game->ttt[1][1] == 'x' && current_game->ttt[1][2] == 'x') ||
					    (current_game->ttt[2][0] == 'x' && current_game->ttt[2][1] == 'x' && current_game->ttt[2][2] == 'x') ||
					    (current_game->ttt[0][0] == 'x' && current_game->ttt[1][0] == 'x' && current_game->ttt[2][0] == 'x') ||
					    (current_game->ttt[0][1] == 'x' && current_game->ttt[1][1] == 'x' && current_game->ttt[2][1] == 'x') ||
					    (current_game->ttt[0][2] == 'x' && current_game->ttt[1][2] == 'x' && current_game->ttt[2][2] == 'x') ||
					    (current_game->ttt[0][0] == 'x' && current_game->ttt[1][1] == 'x' && current_game->ttt[2][2] == 'x') ||
					    (current_game->ttt[0][2] == 'x' && current_game->ttt[1][1] == 'x' && current_game->ttt[2][0] == 'x')) {
						current_game->state = 2;
						sprintf(response, "GAME OVER: %s WON\r\n", current_game->challenger->username);
					}
					else if (current_game->ttt[0][0] != ' ' && current_game->ttt[0][1] != ' ' && current_game->ttt[0][2] != ' ' &&
						 current_game->ttt[1][0] != ' ' && current_game->ttt[1][1] != ' ' && current_game->ttt[1][2] != ' ' &&
						 current_game->ttt[2][0] != ' ' && current_game->ttt[2][1] != ' ' && current_game->ttt[2][2] != ' ') {
						current_game->state = 0;
						sprintf(response, "GAME OVER: DRAW\r\n");	
					}		
					else {
						current_game->state = -1;
						sprintf(response, "%sIN PROGRESS: %s TO MOVE NEXT AS o\r\n", response, current_game->creator->username);
					}
				}

				sprintf( response, "%sa  %c | %c | %c \r\n",response,  current_game->ttt[0][0],  current_game->ttt[0][1],  current_game->ttt[0][2]);
				sprintf( response, "%s  ---|---|---\r\n", response );
				sprintf( response, "%sb  %c | %c | %c \r\n", response, current_game->ttt[1][0],  current_game->ttt[1][1],  current_game->ttt[1][2]);
				sprintf( response, "%s  ---|---|---\r\n", response );
				sprintf( response, "%sc  %c | %c | %c \r\n", response, current_game->ttt[2][0],  current_game->ttt[2][1],  current_game->ttt[2][2]);
				sprintf( response, "%s\r\n", response );
				sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );
			}


		}
		else if( strcmp( command, "LIST" ) == 0 ){ //List the status of all games on the server (both active and completed games)
			if (login_status == 0) 
				strcpy(response, "USER NOT FOUND");
			else if (login_status == 1) {
				game_iterator = game_list_head;
				sprintf(response, "LIST OF GAMES:\r\n");

					while (game_iterator != NULL) {
						sprintf(response, "%sGAME %s: CREATED BY %s, CHALLENGED BY: %s.", 
								response, game_iterator->gamename, game_iterator->creator->username,
								game_iterator->challenger->username);
						if (game_iterator->state == -2) 
							sprintf(response, "%s GAME OVER: %s WON\r\n", 
									response, game_iterator->creator->username);
						else if (game_iterator->state == -1) 
							sprintf(response, "%s IN PROGRESS: %s TO MOVE NEXT AS o\r\n", 
									response, game_iterator->creator->username);
						else if (game_iterator->state == 0)
							sprintf(response, "%s GAME OVER: DRAW\r\n", response);
						else if (game_iterator->state == 1) 
							sprintf(response, "%s IN PROGRESS: %s TO MOVE NEXT AS x\r\n", 
									response, game_iterator->challenger->username);
						else
							sprintf(response, "%s GAME OVER: %s WON\r\n", 
									response, game_iterator->challenger->username);
						game_iterator = game_iterator->next;
					}
				
			}
		}
		else if( strcmp( command, "SHOW" ) == 0 ){ //Show the status of a particular game
			char *game_name = strtok( NULL, ",");

			if( game_name == NULL ){
					sprintf( response, "SHOW COMMAND MUST BE CALLED AS FOLLOWS:\n" );
					sprintf( response+strlen(response), "SHOW,USER,PASS,GAMENAME\n" );
					write(client_sock , response , 2000);  
					close(client_sock);
					continue;
			}

			if (game_list_head == NULL) 
					game_status = 0;
				else {
					game_iterator = game_list_head;
					while (game_iterator != NULL) {
						if (strcmp(game_name, game_iterator->gamename) == 0) {
							game_status = 1;
							current_game = game_iterator;
							break;
						}
						game_iterator = game_iterator->next;
					}
				}

				if (game_status == 0) {
					sprintf(response, "GAME %s DOES NOT EXIST", game_name);
					write(client_sock,response,2000);
					close(client_sock);
					continue;
				}

				sprintf(response, "GAME %s: CREATED BY %s, CHALLENGED BY: %s\r\n", 
						current_game->gamename, current_game->creator->username,
						current_game->challenger->username);
				if (current_game->state == -2) 
					sprintf(response, "%sGAME OVER: %s WON\r\n", 
							response, current_game->creator->username);
				else if (current_game->state == -1) 
					sprintf(response, "%sIN PROGRESS: %s TO MOVE NEXT AS o\r\n", 
							response, current_game->creator->username);
				else if (current_game->state == 0)
					sprintf(response, "%s GAME OVER: DRAW\r\n", response);
				else if (current_game->state == 1) 
					sprintf(response, "%sIN PROGRESS: %s TO MOVE NEXT AS x\r\n", 
							response, current_game->challenger->username);
				else
					sprintf(response, "%sGAME OVER: %s WON\r\n", 
							response, current_game->challenger->username);

				sprintf( response, "%sa  %c | %c | %c \r\n",response,  current_game->ttt[0][0],  current_game->ttt[0][1],  current_game->ttt[0][2]);
				sprintf( response, "%s  ---|---|---\r\n", response );
				sprintf( response, "%sb  %c | %c | %c \r\n", response, current_game->ttt[1][0],  current_game->ttt[1][1],  current_game->ttt[1][2]);
				sprintf( response, "%s  ---|---|---\r\n", response );
				sprintf( response, "%sc  %c | %c | %c \r\n", response, current_game->ttt[2][0],  current_game->ttt[2][1],  current_game->ttt[2][2]);
				sprintf( response, "%s\r\n", response );
				sprintf( response, "%s   %c   %c   %c\r\n", response, '1', '2', '3' );

		}
		else{
	  		sprintf( response, "COMMAND %s NOT IMPLEMENTED", command );
		}

		write(client_sock , response , 2000);  
		close(client_sock);
	}

	close(socket_desc);	
	
	return 0;
}


/**
 * Title: FTP Client
 * Author: Dewanshi Yadav
 * Date: August 12, 2022
*/
// FTP CLIENT
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>
#include "handlers/clienthandler.h"
#include "handlers/porthandler.h"

#define PORT 8080

int main(int argc, char const *argv[])
{
	int sock = 0, read_val, client_fd;
	struct sockaddr_in server_address;

	/**
	 * Referred geeksforgeeks socket programming for implementing socket connection
	 * https://www.geeksforgeeks.org/socket-programming-cc/
	*/

	// Creating client socket file descriptor
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("\nError: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	// assigning address family, ip address and port address
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;

	/**
	 * Connecting the client socket file descriptor `sock` with the `server_address` address
	 */
	if ((client_fd = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address))) < 0)
	{
		perror("\nError: Connection failed\n");
		exit(EXIT_FAILURE);
	}

	/**
	 * Variable declaration for reading/writing user commands and server responses
	 */
	char buffer[200];
	char msg[100];

	int data_fd, data_sock;
	struct sockaddr_in data_address;

	/**
	 * label for reinitializing the user on client side
	 * i.e., user has to login again and has to create a new data connection
	 */
REINITIALIZE:
	while (1)
	{
		memset(msg, 0, 100);
		memset(buffer, 0, 200);

		// Get the user input
		scanf("%[^\n]%*c", msg);
		if (strlen(msg) > 0)
		{
			// Send the user input command to server to perform required action
			send(sock, msg, strlen(msg), 0);

			/**
			 * When user sends `PORT` command to server, client will create
			 * a new data connection for file transfer over this connection
			 */
			if (strstr(msg, "PORT") != NULL)
			{
				// Get the port number from `PORT` command using `parseString` method
				char *port;
				int count = parseString(msg, &port);

				// Check whether the port is provide by the user or not in the command
				if (count > 1)
				{
					/**
					 * Convert port number string `port` to number `port_num`
					 * use this `port_num` for creating new data connection for the particular user
					 * using `dataConnection` method
					 */
					int port_num = atoi(port);
					data_sock = dataConnection(data_fd, data_address, port_num);

					/**
					 * Display the message to the user tjat will tell
					 * data connection status by reading buffer
					 */
					read_val = read(data_sock, buffer, 200);
					buffer[read_val] = '\0';
					printf("Server :: %s\n", buffer);

					/**
					 * loop for scanning the user command after successful
					 * data connection creation
					 */
					while (1)
					{
						memset(msg, 0, 100);
						memset(buffer, 0, 200);

						// Get the user input for file transfer
						scanf("%[^\n]%*c", msg);

						if (strlen(msg) > 0)
						{
							/**
							 * Send the file transfer command provided by the user
							 * to server via data connection
							 */
							send(data_sock, msg, strlen(msg), 0);

							// Retrieve command
							if (strstr(msg, "RETR") != NULL)
							{
								/**
								 * Get `filename` from command string using `parseString` method
								 */
								char *filename;
								int count = parseString(msg, &filename);
								if (count > 1)
								{
									/**
									 * Read the content shared over data connection `data_sock`
									 * from server
									 */
									read_val = read(data_sock, buffer, 200);
									buffer[read_val] = '\0';
									if (buffer != NULL)
									{
										getRETR(buffer, read_val, filename);
									}
								}
							}
							/**
							 * Store command
							 */
							else if (strstr(msg, "STOR") != NULL)
							{
								read_val = read(data_sock, buffer, 200);
								buffer[read_val] = '\0';
								if (buffer != NULL)
								{
									/**
									 * Get `filename` from command string using `parseString` method
									 * then call the `getSTOR`method for obtaining the content of given file
									 * from client and to send it to the server via `data_sock`
									 */
									char *filename;
									int count = parseString(msg, &filename);
									if (count > 1)
									{
										char buff[200];
										int n = getSTOR(buff, filename);
										if (n > 0)
										{
											send(data_sock, buff, strlen(buff), 0);
										}
										read_val = read(data_sock, buffer, 200);
										buffer[read_val] = '\0';
										printf("Server :: %s\n", buffer);
									}
								}
							}
							/**
							 * Append the file content
							 */
							else if (strstr(msg, "APPE") != NULL)
							{
								read_val = read(data_sock, buffer, 200);
								buffer[read_val] = '\0';
								printf("buffer: %s\n", buffer);
								if (buffer != NULL)
								{
									/**
									 * Get `filename` from command string using `parseString` method
									 * then call the `getAPPE`method for obtaining the content of given file
									 * from client and to send it to the server via `data_sock` for appending
									 * to file
									 */
									char *filename;
									int count = parseString(msg, &filename);
									if (count > 1)
									{
										char buff[200];
										int n = getAPPE(buff, filename);
										if (n > 0)
										{
											send(data_sock, buff, strlen(buff), 0);
										}
										read_val = read(data_sock, buffer, 200);
										buffer[read_val] = '\0';
										printf("Server :: %s\n", buffer);
									}
								}
							}
							/**
							 * Quit command will terminated the data and communication connection
							*/
							else if (strcmp(msg, "QUIT") == 0)
							{
								read_val = read(data_sock, buffer, 200);
								buffer[read_val] = '\0';
								printf("Server :: %s\n", buffer);
								close(data_sock);
								close(client_fd);
								char *conn = "connection close.\n";
								send(sock, conn, strlen(conn), 0);
								exit(0);
							}

							/**
							 * Rename from is used for checking whether the given filename
							 * exists in server or not and if it is there then the filename 
							 * will be rename using `RNTO` command sent by user
							*/
							else if (strstr(msg, "RNFR") != NULL)
							{
								read_val = read(data_sock, buffer, 200);
								buffer[read_val] = '\0';
								printf("Server :: %s\n", buffer);

								memset(msg, 0, 100);
								scanf("%[^\n]%*c", msg);

								send(data_sock, msg, strlen(msg), 0);

								memset(buffer, 0, 200);
								read_val = read(data_sock, buffer, 200);
								buffer[read_val] = '\0';
								printf("Server :: %s\n", buffer);
							}

							/**
							 * Abort - terminate the user
							 * by closing the data connection
							*/
							else if (strcmp(msg, "ABOR") == 0)
							{
								read_val = read(data_sock, buffer, 200);
								buffer[read_val] = '\0';
								printf("Server :: %s\n", buffer);
								close(data_sock);
								close(data_fd);
								shutdown(data_fd, SHUT_RDWR);
								break;
							}

							/**
							 * Reinitialize the user
							 * Close the data connection socket and data socket file descriptor
							 * that will logout the user and leaving the control connection unaffected
							 * then jump to label `REINITIALIZE`
							*/
							else if (strcmp(msg, "REIN") == 0)
							{
								read_val = read(data_sock, buffer, 200);
								buffer[read_val] = '\0';
								printf("Server :: %s\n", buffer);
								close(data_sock);
								close(data_fd);
								shutdown(data_fd, SHUT_RDWR);
								goto REINITIALIZE;
							}

							/**
							 * If no action is required at client side for given command
							 * then directly read the content or message shared by server over
							 * `data_sock` and display it to the user
							 */
							else
							{
								read_val = read(data_sock, buffer, 200);
								buffer[read_val] = '\0';
								printf("Server :: %s\n", buffer);
							}
						}
					}
					// Close the data connection socket and data socket fd
					close(data_sock);
					close(data_fd);
				}
			}
			/**
			 * If no action is required at client side for given command
			 * then directly read the content or message shared by server over
			 * `sock` (control connection) and display it to the user
			 */
			else
			{
				read_val = read(sock, buffer, 200);
				buffer[read_val] = '\0';
				printf("Server :: %s\n", buffer);
			}
		}
	}

	// Closing the connected socket
	close(sock);
	close(client_fd);
	shutdown(sock, SHUT_RDWR);
	return 0;
}
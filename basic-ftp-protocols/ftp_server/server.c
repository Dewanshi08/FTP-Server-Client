/**
 * Title: FTP Server
 * Author: Dewanshi Yadav
 * Date: August 12, 2022
*/
// FTP Server
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include "handlers/serverhandler.h"
#include "handlers/replycodes.h"

#define PORT 8080
#define MAXSIZE 200

// void closeServerConn()
// {
// 	send(data_socket, data_conn_close, strlen(data_conn_close), 0);
// 	close(data_socket);
// 	close(server_socket);
// 	close(server_fd);
// 	shutdown(server_fd, SHUT_RDWR);
// 	exit(0);
// }

/**
 * This method identifies the command send from the client side by the user
 * and executes the command
*/
void commandHandler(char *cmd, int server_socket, int data_sock, int server_fd, char *home_dir)
{
	printf("Client :: %s\n", cmd);

	/**
	 * `CWD` -> Change Working Directory
	*/
	if (strstr(cmd, "CWD") != NULL)
	{
		char cwd[MAXSIZE];
		getCWD(cmd, cwd);
		send(data_sock, cwd, strlen(cwd), 0);
	}
	/**
	 * `CDUP` -> Change to Parent Directory
	*/
	else if (strcmp(cmd, "CDUP") == 0)
	{
		char cdup[MAXSIZE];
		int flag = getCDUP(cdup, home_dir);
		if (flag == 0)
			send(data_sock, command_execution_fail, strlen(command_execution_fail), 0);
		else
			send(data_sock, cdup, strlen(cdup), 0);
	}
	/**
	 * `REIN` -> Reinitialize the user by closing the data connection
	 * and terminates the user from control connection
	 * `rein_flag` is set for goto label
	*/
	else if (strcmp(cmd, "REIN") == 0)
	{
		send(data_sock, connection_successful, strlen(connection_successful), 0);
		rein_flag = 1;
	}
	/**
	 * `QUIT` -> This command terminates the user
	*/
	else if (strcmp(cmd, "QUIT") == 0)
	{
		send(data_sock, data_conn_close, strlen(data_conn_close), 0);
		close(data_sock);
		char conn[MAXSIZE];
		read(server_socket, conn, MAXSIZE);
		close(server_socket);
		shutdown(server_fd, SHUT_RDWR);
		exit(0);
	}
	/**
	 * `RETR` -> Download file from server to client
	*/
	else if (strstr(cmd, "RETR") != NULL)
	{
		if(getRETR(cmd, data_sock) == 0)
			send(data_sock, bad_command, strlen(bad_command), 0);
	}
	/**
	 * `STOR` -> Upload file to server from client
	*/
	else if (strstr(cmd, "STOR") != NULL)
	{
		char *filename;
		int count = parseString(cmd, &filename);
		if (count > 1)
		{
			if (getSTOR(data_sock, filename) > 0)
				send(data_sock, file_action_completed, strlen(file_action_completed), 0);

			else
				send(data_sock, command_execution_fail, strlen(command_execution_fail), 0);
		}
		else
			send(data_sock, bad_command, strlen(bad_command), 0);
	}
	/**
	 * `APPE` -> Append the content on the file from client to server
	*/
	else if (strstr(cmd, "APPE") != NULL)
	{
		char *filename;
		int count = parseString(cmd, &filename);
		if (count > 1)
		{
			if (getAPPE(data_sock, filename) > 0)
				send(data_sock, file_action_completed, strlen(file_action_completed), 0);

			else
				send(data_sock, command_execution_fail, strlen(command_execution_fail), 0);
		}
		else
			send(data_sock, bad_command, strlen(bad_command), 0);
	}
	/**
	 * `RNFR` -> Rename file from server
	*/
	else if (strstr(cmd, "RNFR") != NULL)
	{
		int c = getRNFR(cmd, data_sock);
		printf("%d\n", c);
		if (c == 1)
			send(data_sock, file_action_completed, strlen(file_action_completed), 0);
		else
			send(data_sock, command_execution_fail, strlen(command_execution_fail), 0);
	}
	/**
	 * `RNTO` -> Rename file to server
	*/
	else if (strstr(cmd, "RNTO") != NULL)
	{
		send(data_sock, command_execution_fail, strlen(command_execution_fail), 0);
	}
	/**
	 * `ABOR` -> Abort any ftp service command and 
	 * close the data connection
	*/
	else if (strstr(cmd, "ABOR") != NULL)
	{
		send(data_sock, connection_abort, strlen(connection_abort), 0);
		data_conn_flag = 1;
	}
	/**
	 * `DELE` -> Delete the specified file from server if it exists
	*/
	else if (strstr(cmd, "DELE") != NULL)
	{
		int deleFlag = getDELE(cmd, data_sock);
		if (deleFlag)
			send(data_sock, file_action_completed, strlen(file_action_completed), 0);
		else
			send(data_sock, command_execution_fail, strlen(command_execution_fail), 0);
	}
	/**
	 * `RMD` -> Remove specified directory from the server if it exists
	*/
	else if (strstr(cmd, "RMD") != NULL)
	{
		int rmdFlag = getRMD(cmd, data_sock);
		if (rmdFlag)
			send(data_sock, file_action_completed, strlen(file_action_completed), 0);
		else
			send(data_sock, command_execution_fail, strlen(command_execution_fail), 0);
	}
	/**
	 * `MKD` -> Create new directory on server
	*/
	else if (strstr(cmd, "MKD") != NULL)
	{
		int mkdFlag = getMKD(cmd, data_sock);
		if (mkdFlag)
			send(data_sock, file_action_completed, strlen(file_action_completed), 0);
		else
			send(data_sock, directory_already_exists, strlen(directory_already_exists), 0);
	}
	/**
	 * `PWD` -> Get the current working directory in server
	*/
	else if (strcmp(cmd, "PWD") == 0)
	{
		char pwd[50];
		getPWD(pwd);
		send(data_sock, pwd, strlen(pwd), 0);
	}
	/**
	 * `LIST` -> Get the list of files and directories for specified path
	 * in server
	*/
	else if (strstr(cmd, "LIST") != NULL)
	{
		char *file;
		int count = parseString(cmd, &file);
		getLIST(cmd, data_sock, file, count);
	}
	// else if (strstr(cmd, "STAT") != NULL)
	// {
	// }
	/**
	 * `NOOP` -> No action needed
	*/
	else if (strstr(cmd, "NOOP") != NULL)
	{
		send(data_sock, noop, strlen(noop), 0);
	}
	/**
	 * If command does not match from the above list of command 
	 * send a message to user specifying that command does not exist
	*/
	else
	{
		send(data_sock, bad_command, strlen(bad_command), 0);
	}
}

int main(int argc, char const *argv[])
{
	// Make the current working directory as server home directory 
	char home_dir[MAXSIZE];
	getcwd(home_dir, MAXSIZE);

	/**
	 * If directory name is specified at the time of execution
	 * change the working directory to specified location
	*/
	if (argc == 2)
	{		
		chdir(argv[1]);
		getcwd(home_dir, MAXSIZE);
	}

	/**
	 * Referred geeksforgeeks socket programming for implementing socket connection
	 * https://www.geeksforgeeks.org/socket-programming-cc/
	*/

	int server_fd, server_socket, read_val;
	struct sockaddr_in server_address;
	int opt = 1;

	// Creating server socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("\nError: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	// assigning address family, ip address and port address

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(PORT);

	// binding the server socket file descriptor `server_fd` with `server_address`
	if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		perror("\nError: Binding failed\n");
		exit(EXIT_FAILURE);
	}

	/**
	 * listen will start listening to incoming connection requests to `server_fd`
	 * here `5` is backlog which defines the number of incoming connection requests that can be accepted
	 */
	if (listen(server_fd, 5) < 0)
	{
		perror("\nError: Listening failed\n");
		exit(EXIT_FAILURE);
	}

	printf("Server is listening...\n");

	/**
	 * contiuously accepting the new connection requests made to `server_fd`
	 * from clients
	 */
	while (1)
	{
		struct sockaddr_in client_address;
		int addrlen = sizeof(client_address);

		/**
		 * accepting the connection request made by client on `server_fd`
		 * and returning the file descriptor for that socket i.e.,
		 * stored in `server_socket`
		 */

		if ((server_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0)
		{
			perror("\nError: Accept failed\n");
			exit(EXIT_FAILURE);
		}

		// Forking a child process
		int pid = fork();
		if (pid < 0)
		{
			perror("\nError:  Fork failed\n");
			break;
		}

		// Child Process
		if (pid == 0)
		{
			// signal(SIGINT, closeServerConn);
			close(server_fd);

			char buffer[MAXSIZE] = "";

			// loop for executing multiple commands requested by client for user login
			while (1)
			{
			/**
			 * label for reinitializing the user on client side
			 * i.e., user has to login again and has to create a new data connection
			 */
			REINITIALIZE:

				/**
				 * Read incoming command request from client side on control connection `server_socket`
				 * login attemp will be only successful when the user sends `USER` command
				 * otherwise server will send a `login_fail` message on control connection
				 */
				read_val = read(server_socket, buffer, MAXSIZE);
				buffer[read_val] = '\0';
				if (strcmp(buffer, "USER") == 0)
				{
					send(server_socket, success_login, strlen(success_login), 0);
					printf("%s\n", success_login);
					break;
				}
				else
					send(server_socket, login_fail, strlen(login_fail), 0);
			}

			int data_socket;
		/**
		 * label for creating new data connection once the connection is closed by the `USER`
		 * on both client and server site
		 */
		DATA_CONNECTION:
			// loop for executing multiple commands requested by client for data connection creation
			while (1)
			{
				// clearing the buffer content using memset
				memset(buffer, 0, MAXSIZE);

				/**
				 * Read incoming command request from client side on control connection `server_socket`
				 * data connection will be successful when the user sends `PORT` command with a port address
				 * otherwise server will send a `data_conn_fail` message on control connection
				 * incase of missing port address `bad_command` message will be sent
				 */
				read_val = read(server_socket, buffer, MAXSIZE);
				buffer[read_val] = '\0';

				if (strstr(buffer, "PORT") != NULL)
				{
					// Getting the port number from `PORT` command
					char *port;
					int count = parseString(buffer, &port);
					if (count > 1)
					{
						// Converting port number string to number
						int port_num = atoi(port);

						/**
						 * Calling the getPort method in serverhandler for creating a new data connection
						 * on requested port address by the user and returning the file descriptor of
						 * data connection socket `data_socket`
						 */
						data_socket = getPORT(port_num, server_socket, client_address);
						// Sending the message to user on client site of successful data connection created
						send(data_socket, data_conn_success, strlen(data_conn_success), 0);
						break;
					}
					else
						send(server_socket, bad_command, strlen(bad_command), 0);
				}
				else
					send(server_socket, data_conn_fail, strlen(data_conn_fail), 0);
			}

			// loop for executing multiple commands requested by client after successful data connection
			while (1)
			{
				// clearing the buffer content using memset
				memset(buffer, 0, MAXSIZE);
				/**
				 * Read incoming command request from client side on data connection `data_socket`
				 * call the `commandHandler` method for executing commands accordingly (which are implemented on server)
				 */
				read_val = read(data_socket, buffer, MAXSIZE);
				buffer[read_val] = '\0';

				commandHandler(buffer, server_socket, data_socket, server_fd, home_dir);

				/**
				 * `data_conn_flag` will notify the server that data connection is for the user
				 * and it will jump to label `DATA_CONNECTION` for restarting the data connection
				 * on new port address provided by the user
				 */
				if (data_conn_flag == 1)
				{
					data_conn_flag = 0;
					goto DATA_CONNECTION;
				}
				/**
				 * `rein_falg` will notify the server that user has been logged out
				 * and it will jump to label `REINITIALIZE` through which user can login
				 * to the existing control connection
				 */
				else if (rein_flag == 1)
				{
					rein_flag = 0;
					goto REINITIALIZE;
				}
			}

			/**
			 * closing the data connection socket `data_socket`,
			 * control connection socket `server_socket` and
			 * server socket file descriptor `server_fd`
			 */
			close(data_socket);
			close(server_socket);
			close(server_fd);
		}
	}
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}
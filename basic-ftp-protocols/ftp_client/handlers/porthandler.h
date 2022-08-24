/**
 * Title: FTP Client Data Connection
 * Author: Dewanshi Yadav
 * Date: August 12, 2022
*/
#include <sys/socket.h>
#include <stdlib.h>

// This method is used for creating a data connection
int dataConnection(int data_fd, struct sockaddr_in data_address, int port)
{
	int data_sock;

	/**
	 * Referred geeksforgeeks socket programming for implementing socket connection
	 * https://www.geeksforgeeks.org/socket-programming-cc/
	*/

	// Creating data socket file descriptor
	if ((data_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("\nError: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	// assigning address family, ip address and port address
	data_address.sin_family = AF_INET;
	data_address.sin_addr.s_addr = INADDR_ANY;
	data_address.sin_port = htons(port);

	// binding the data socket file descriptor `data_fd` with `data_address`
	if (bind(data_fd, (struct sockaddr *)&data_address, sizeof(data_address)) < 0)
	{
		perror("\nError: Binding failed\n");
		exit(EXIT_FAILURE);
	}

	/**
	 * listen will start listening to incoming connection requests to `data_fd`
	 * here `5` is backlog which defines the number of incoming connection requests that can be accepted
	 */
	if (listen(data_fd, 5) < 0)
	{
		perror("\nError: Listening failed\n");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in data_address1;
	int addrlen = sizeof(data_address1);

	/**
	 * accepting the connection request made by server on `data_fd`
	 * and returning the file descriptor for that socket i.e.,
	 * stored in `data_sock`
	 */

	if ((data_sock = accept(data_fd, (struct sockaddr *)&data_address1, (socklen_t *)&addrlen)) < 0)
	{
		perror("\nError: Accept failed\n");
		exit(EXIT_FAILURE);
	}
	return data_sock;
}
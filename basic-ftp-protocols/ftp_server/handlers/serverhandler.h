/**
 * Title: FTP Server Handler Functions
 * Author: Dewanshi Yadav
 * Date: August 12, 2022
*/
#include <dirent.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
#define _XOPEN_SOURCE_EXTENDED 1
#include <sys/socket.h>
// #include "handlers/replycodes.h"

#define MAXSIZE 200

// Variable declaration
int data_conn_flag, rein_flag;

/**
 * This method obtains the argument from the command
 * send from the client using `strsep` method and returns the count
 */
int parseString(char *str, char **argv)
{
	char *chunk;
	int count = 0;
	char *dupStr = strdup(str);
	while ((chunk = strsep(&dupStr, " ")))
	{

		if (strlen(chunk) != 0 && chunk != NULL)
		{
			if (count == 1)
			{
				*argv = strdup(chunk);
			}
			count++;
		}
	}
	free(dupStr);
	return count;
}

/**
 * Change Working Directory
 * This method obtains the argument (directory) from the command
 * send from the client using `parseString` method
 */
void getCWD(char *cmd, char *cwd)
{
	char *path;
	int count = parseString(cmd, &path);
	/**
	 * if the command does not contain directory name (i.e., `count` is `1`)
	 * then server will get current working directory
	 */
	if (count == 1)
	{
		getcwd(cwd, MAXSIZE);
	}
	/**
	 * if the command contains directory name then server will change the directory
	 * using `chdir` method and get new current working directory
	 */
	else
	{
		chdir(path);
		getcwd(cwd, MAXSIZE);
	}
}

/**
 * Change to Parent Directory
 *
 */
int getCDUP(char *cdup, char *home_directory)
{
	/**
	 * Get the current working directory of server that is been accessed by user
	 * if the current working directory `cdup` does not matches with server home directory
	 * then user is allow to go back to the parent directory
	 */
	getcwd(cdup, MAXSIZE);
	if (strcmp(cdup, home_directory))
	{
		char *path = "../";
		chdir(path);
		getcwd(cdup, MAXSIZE);
		return 1;
	}
	/**
	 * if the current working directory `cdup` matches with server home directory
	 * then user is not allow to go back to the parent directory
	 */
	else
	{
		return 0;
	}
}

/**
 * Create Data Connection using PORT command
 */
int getPORT(int port, int sock, struct sockaddr_in data_addr)
{
	/**
	 * Referred geeksforgeeks socket programming for implementing socket connection
	 * https://www.geeksforgeeks.org/socket-programming-cc/
	*/

	struct sockaddr_in data_server;

	// Creating a data socket file descriptor
	int data_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Cleaning the content of `data_server` object if any garbage value is present
	memset((char *)&data_server, 0, sizeof(data_server));

	/**
	 * assigning address family, ip address (same as control connection ip)
	 * and user specified port address
	 */
	data_server.sin_family = AF_INET;
	data_server.sin_addr.s_addr = data_addr.sin_addr.s_addr;
	data_server.sin_port = htons(port);

	/**
	 * Connecting the data socket file descriptor `data_sock` with the `data_server` address
	 */
	if (connect(data_sock, (struct sockaddr *)&data_server, sizeof(data_server)) == -1)
	{
		printf("Error: %d\n", errno);
		return 0;
	}
	else
	{
		return data_sock;
	}
}

/**
 * Retrieve file from server to client
 */
int getRETR(char *cmd, int data_sock)
{
	/**
	 * Get `filename` from command string using `parseString` method
	 */
	char *filename;
	int count = parseString(cmd, &filename);

	/**
	 * If `filename` is given by the user (i.e., count > 1)
	 * then open the file and get the file descriptor `fd` use this `fd` for reading the file
	 * and send the buffer `buff` to the user
	 */
	if (count > 1)
	{
		int fd = open(filename, O_RDWR);

		char buff[MAXSIZE] = "";
		int n = read(fd, buff, MAXSIZE);
		buff[n] = '\0';

		send(data_sock, buff, strlen(buff), 0);
		printf("Send the file\n");
		close(fd);
		return 1;
	}
	else
		return 0;
}

/**
 * Store the file to server (upload) from client side
 */
int getSTOR(int data_sock, char *filename)
{
	char *check = "Waiting...";
	write(data_sock, check, strlen(check));

	/**
	 * Read file data send by user in data socket file descriptor
	 */
	char buffer[MAXSIZE];
	int read_val = read(data_sock, buffer, MAXSIZE);
	buffer[read_val] = '\0';

	/**
	 * If the filename already exist then remove the file in server
	 */
	if (access(filename, F_OK) == 0)
	{
		remove(filename);
	}
	/**
	 * Create new file with specified `filename` in server and
	 * get the `fd` of the file for writting the content from `buffer`
	 * to the file in server
	 */
	int fd = open(filename, O_CREAT | O_WRONLY, 0777);
	int n = write(fd, buffer, read_val);
	return n;
}

/**
 * Append the content of file from client to same file in server
 */
int getAPPE(int data_sock, char *filename)
{
	char buffer[500];
	int fd, n, read_val;
	char *check = "Waiting...";

	/**
	 * Read file data send by user in data socket file descriptor
	 */
	send(data_sock, check, strlen(check), 0);
	read_val = read(data_sock, buffer, 500);
	buffer[read_val] = '\0';

	/**
	 * If the filename already exist then open the file get the `fd`
	 * set the offset at the end of the file
	 * then write the new content to the file in server
	 */
	if (access(filename, F_OK) == 0)
	{
		fd = open(filename, O_RDWR);
		lseek(fd, 0, SEEK_END);
		n = write(fd, buffer, read_val);
	}
	/**
	 * If file does not exist then create new file in server
	 * and write the content in this file
	 */
	else
	{
		fd = open(filename, O_CREAT | O_WRONLY, 0777);
		n = write(fd, buffer, read_val);
	}
	return n;
}

/**
 * Rename the given filename to new filename provide by user
 */
int getRNFR(char *cmd, int data_sock)
{
	/**
	 * Get `filename` from command string using `parseString` method
	 */
	char *filename;
	int count = parseString(cmd, &filename);

	/**
	 * If `filename` is not null and `filename` exists in the server
	 * then proceed further for chnaging the filename
	 */
	if (filename != NULL && (access(filename, F_OK) == 0))
	{
		/**
		 * Get the content of `filename` store it in buffer `buffer1`
		 * and remove the file using remove method
		 */
		char buffer1[MAXSIZE];
		int fd1 = open(filename, O_RDWR);
		int read_val = read(fd1, buffer1, MAXSIZE);
		buffer1[read_val] = '\0';

		remove(filename);

		/**
		 * Inform user to provide the new filename
		 */
		char *rename_to = "Enter rename to...";
		send(data_sock, rename_to, strlen(rename_to), 0);

		/**
		 * Read the user command (RNTO) and argument (NEW_FILENAME)
		 */
		char buffer2[MAXSIZE];
		int n = read(data_sock, buffer2, MAXSIZE);
		buffer2[n] = '\0';
		printf("%s\n", buffer2);

		/**
		 * If the command matches with RNTO then get new filename `rnto_filename`
		 */
		if (strstr(buffer2, "RNTO") != NULL)
		{
			char *rnto_filename;
			int count = parseString(buffer2, &rnto_filename);

			/**
			 * Check whether user has provided the new filename or not `rnto_filename`
			 * using `parseString` method
			 * create new file in server, write the content from `buffer1` to `rnto_filename`
			 */
			if (count > 1)
			{
				int fd2 = open(rnto_filename, O_CREAT | O_WRONLY, 0777);
				write(fd2, buffer1, read_val);
				return 1;
			}
		}
	}
	return 0;
}

/**
 * Delete specified from server
 */
int getDELE(char *cmd, int data_sock)
{
	/**
	 * Get the `filename` from the user command using `parseString` method
	 */
	char *filename;
	int count = parseString(cmd, &filename);

	/**
	 * Check whether given `filename` exists in server or not
	 * if it exist then remove the file and send the success message
	 * to the user
	 */
	if (count > 1 && access(filename, F_OK) == 0)
	{
		remove(filename);
		char *msg = "Successfully Deleted the file.\n";
		send(data_sock, msg, strlen(msg), 0);
		return 1;
	}
	return 0;
}

/**
 * Remove specified directory from the server
 */
int getRMD(char *cmd, int data_sock)
{
	/**
	 * Get the `directory` from the user command using `parseString` method
	 */
	char *directory;
	int count = parseString(cmd, &directory);

	/**
	 * Check whether given `directory` exists in server or not
	 * if it exist and it does not contain any files or subdirectories
	 * then remove the directory and send the success message
	 * to the user
	 */
	DIR *dir = opendir(directory);
	if (count > 1 && dir == NULL)
	{

		rmdir(directory);
		char *msg = "Successfully Deleted the directory.\n";
		send(data_sock, msg, strlen(msg), 0);
		return 1;
	}
	return 0;
}

/**
 * Create new directory in the server
 */
int getMKD(char *cmd, int data_sock)
{
	/**
	 * Get the `directory` from the user command using `parseString` method
	 */
	char *directory;
	int count = parseString(cmd, &directory);

	/**
	 * If directory already exists then send the message to user accordingly
	 */
	if (count < 2 && access(directory, F_OK) == 0)
	{
		return 0;
	}
	/**
	 * If directory does not exists then create new directory using mkdir method
	 */
	else
	{
		int fd = mkdir(directory, 0777);
		return 1;
	}
}

/**
 * Get the current working directory using getcwd method
 */
void getPWD(char *pwd)
{
	getcwd(pwd, MAXSIZE);
}

/**
 * Get the list of files and directories present in specified path in server
 */
void getLIST(char *cmd, int data_sock, char *file, int count)
{
	char list[MAXSIZE] = "";
	DIR *dp;
	struct dirent *dirp;

	/**
	 * Get the current working directory `path` of server
	 */
	char path[MAXSIZE];
	getcwd(path, MAXSIZE);

	/**
	 * Add the NEW_PATH provided by user to obtain the list of files and directories
	 * to `path` and open the directory using opendir method
	 */
	if (count > 1)
	{
		strcat(path, "/");
		strcat(path, file);
	}

	/**
	 * Referred Chapter 1 for getting the list of all the files in a directory
	 * from slide 23
	*/

	dp = opendir(path);

	/**
	 * Get the directory structure object using readdir method
	 * from that fetch the name of file or directory and a create list
	 */
	while ((dirp = readdir(dp)) != NULL)
	{
		char *temp = dirp->d_name;
		if (strcmp(temp, ".") && strcmp(temp, ".."))
		{
			strcat(temp, "\t");
			strcat(list, temp);
		}
		temp = "";
	}
	/**
	 * Send the list to the user and close the file directory pointer using closedir method
	 */
	if (strlen(list) > 0)
		send(data_sock, list, strlen(list), 0);
	else
	{
		char *empty_list = "List is empty.";
		send(data_sock, empty_list, strlen(empty_list), 0);
	}
	closedir(dp);
}
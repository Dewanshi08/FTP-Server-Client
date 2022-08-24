/**
 * Title: FTP Client Handler Functions
 * Author: Dewanshi Yadav
 * Date: August 12, 2022
*/
#include <dirent.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAXSIZE 100

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
	return count;
}

/**
 * This method is used for writing the content to given `filename` in client
 * received from server
 */
void getRETR(char *buffer, int valread, char *filename)
{
	// Get the current working directory of client side
	char path[100];
	getcwd(path, 100);

	// Concate the new filename to the path
	strcat(path, "/");
	strcat(path, filename);

	// Get the `fd` for `path` and write the content to the file
	int fd = open(path, O_WRONLY | O_CREAT, 0777);
	write(fd, buffer, valread);
	printf("File received\n");
}

/**
 * This method is used for reading the content of `filename` from client side
 * which will be further used in calling process for writing content to
 * new file in server
 */
int getSTOR(char *buffer, char *filename)
{
	char path[100];
	int fd = open(filename, O_RDONLY);
	int n = read(fd, buffer, 200);
	buffer[n] = '\0';
	return n;
}

/**
 * This method is used for reading the content of `filename` from client side
 * which will be further used in calling process for appending content to
 * the file in server
 */
int getAPPE(char *buffer, char *filename)
{
	char path[100];
	int fd = open(filename, O_RDONLY);
	int n = read(fd, buffer, 200);
	buffer[n] = '\0';
	return n;
}

/**
 * Referred below mentioned link for obtaining the reply codes for FTP
 * https://www.w3.org/Protocols/rfc959/
 */
static char *success_login = "230 User logged in, proceed.";
static char *login_fail = "530 Not logged in.";
static char *connection_successful = "200 Command okay.";
static char *connection_failure = "425 Can't open data connection.";
static char *bad_command = "503 Bad sequence of commands.";
static char *connection_close = "226 Closing data connection.";
static char *connection_abort = "426 Connection closed; transfer aborted.";
static char *command_execution_fail = "550 Requested action not taken.";
static char *file_action_completed = "250 Requested file action okay, completed.";
static char *directory_already_exists = "521 directory already exists; taking no action.";
static char *data_conn_success = "225 Data connection open; no transfer in progress.";
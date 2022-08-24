/**
 * Title: FTP Server Reply Codes
 * Author: Dewanshi Yadav
 * Date: August 12, 2022
*/
/**
 * Referred below mentioned link for obtaining the reply codes for FTP
 * https://www.w3.org/Protocols/rfc959/
 */

/**
 * Messages conatining reply codes for actions taken at server side
 */

static char *success_login = "230 User logged in, proceed.";
static char *login_fail = "530 Not logged in.";
static char *connection_successful = "200 Command okay.";
static char *connection_failure = "425 Can't open data connection.";
static char *bad_command = "502 Command not implemented.";
static char *connection_close = "221 Service closing control connection.";
static char *connection_abort = "426 Connection closed; transfer aborted.";
static char *command_execution_fail = "550 Requested action not taken.";
static char *file_action_completed = "250 Requested file action okay, completed.";
static char *directory_already_exists = "521 directory already exists; taking no action.";
static char *rnto = "Enter the new filename for rename.";
static char *data_conn_success = "225 Data connection open; no transfer in progress.";
static char *data_conn_fail = "425 Can't open data connection.";
static char *data_conn_close = "226 Closing data connection.";
static char *noop = "OK.";
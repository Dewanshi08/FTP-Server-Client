# FTP-Server-Client
 Basic ftp server client protocol using C

 List of commands supported by the program are
 1. LIST
 2. CWD
 3. PWD
 4. CDUP
 5. STOR
 6. RETR
 7. APPE
 8. DELE
 9. MKD
 10. RMD
 11. RNFR
 12. RNTO
 13. QUIT
 14. ABOR
 
 After executing the server and client object files control connection will be established.
 Now, server will start listening to the commands or instructions send from the client.
 STEP 1: To be able to execute the commands user must login using 
 `USER` command.
   Server will send a response informing the user whether 
   login is successful or not.
 STEP 2: User should provide a port number using `PORT` command.
   Server will create a data connection for file transfer using the 
   `PORT` command on given port address.
 STEP 3: Now the user can send any command from the above command list
 along with the parameters if needed.
   Server will send response to the user accordingly.

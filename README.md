## File Transfer System
#### Created by Richard Joseph
#### Languages: C/C++ Server and Python Client
> #### Sources: 
> - http://www.linuxhowtos.org/C_C++/socket.htm 
> - https://docs.python.org/3.8/howto/sockets.html
> - https://wiki.python.org/moin/TcpCommunication
> - https://github.com/gregmankes/cs372-project2/blob/master/ftclient.py
> - http://www.bogotobogo.com/python/python_network_programming_server_client_file_transfer.php
> - https://mail.python.org/pipermail/tutor/2008-September/064201.html

#### Instructions:
1. Compile the cpp file by typing in the command line 'gcc ftserver.c -o ftserver'.
2. Run the following command on the **flip1** server './ftserver <Port #>'.
3. Run ftclient.py on flip2 by typing in the command line 'python3 ftclient.py <Server Name> <Server Port #> -l <Data Port #>'.
4. OR Run ftclient.py on flip2 by typing in the command line 'python3 ftclient.py <Server Name> <Server Port #> -g <file_name> <Data Port #>'.
5. ftclient.py will terminate after it is finished to exit ftserver.c you must end it by pressing CTRL + C on keyboard.

Note: Will only work on flip servers.
    



# Socket-Programming

This repository is for assignment of my Internet Engineering course which includes some practices on socket programming
In this assignment, I enhance a simple web-server.
There is a simple sample HTTP server code (SimpleHTTPServer.c) and a client code
(clien.c). And four different servers or clients are implemented as described below:
1. "server1" is a multi-process server that forks a process for every new request it receives.
2. "server2" is a single process server that uses the "select" system call to handle multiple clients. Again, much like server1, server2 is also be able to handle multiple requests concurrently.
3. “server3” and client program “client1” goes as follows:
The server is a dummy math server that could solve simple arithmetic operations
simultaneously and client is multi-threaded.
4. Finally, multiple clients can be able to connect and chat with the server at the same time,
causing no problems.
<br>

Also You can check : <a href="https://github.com/ghazalb76/SocketProgramming">Hoda Farsi Digit Dataset</a>

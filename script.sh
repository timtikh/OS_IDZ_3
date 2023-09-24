gcc TCPLib.c Client.c -o Client
gcc TCPLib.c Server.c -o Server
./Server 5000 5
./Client 1 localhost 5000
gcc TCPLib.c Client.c -o Client
gcc TCPLib.c Server.c -o Server
./Server 5000 5
./Client 1 127.0.0.1 5000
./Client 2 127.0.0.1 5000
./Client 3 127.0.0.1 5000
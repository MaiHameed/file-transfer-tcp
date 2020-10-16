This is an example application that showcases a file download between a server and a client using a TCP connection. This is a C 
application.


# How to run

This application needs to be run between two hosts on the same network. I used two separate VMs on the same computer. 
`git clone` this repository to both hosts.

## Set up the server host
1. Switch to the `server` branch on the server acting host `git checkout server`
2. Compile the server application with GCC using `gcc -o tcpServer tcpServer.c -lnsl`

## Set up the client host
1. Switch to the `client` branch on the client acting host `git checkout client`
2. Compile the client application with GCC using `gcc -o tcpClient tcpClient.c -lnsl`
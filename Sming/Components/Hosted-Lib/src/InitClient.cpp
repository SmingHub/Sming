#include <SmingCore.h>
#include <HostedClient.h>
#include <HostedClientStream.h>


// TODO: Initialize the code transport and the client

HostedClient hostedClient(new HostedTcpStream("192.168.13.3", 4031));

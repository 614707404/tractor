#include <tractor/TcpConnection.h>
#include <tractor/SockAddr.h>
#include <tractor/EventLoop.h>
#include <tractor/TcpServer.h>
void onConnection(const tractor::TcpConnection::TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toString().c_str());
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }
}

void onMessage(const tractor::TcpConnection::TcpConnectionPtr &conn,
               const char *data,
               ssize_t len)
{
    printf("onMessage(): received %zd bytes from connection [%s]\n",
           len, conn->name().c_str());
}
int main()
{
    printf("main(): pid = %d\n", getpid());

    tractor::SockAddr listenAddr(9981);
    tractor::EventLoop loop;

    tractor::TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}
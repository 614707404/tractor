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
               tractor::Buffer *buf,
               int64_t receiveTime)
{
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->readableBytes(),
           conn->name().c_str(),
           std::to_string(receiveTime).c_str());

    printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
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
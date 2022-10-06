#include "tractor/TcpServer.h"
#include "tractor/EventLoop.h"
#include "tractor/SockAddr.h"
#include <stdio.h>

std::string message;

void onConnection(const tractor::TcpConnection::TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toString().c_str());
        conn->send(message);
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }
}

void onWriteComplete(const tractor::TcpConnection::TcpConnectionPtr &conn)
{
    conn->send(message);
}

void onMessage(const tractor::TcpConnection::TcpConnectionPtr &conn,
               tractor::Buffer *buf,
               int64_t receiveTime)
{
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->readableBytes(),
           conn->name().c_str(),
           std::to_string(receiveTime).c_str());

    buf->retrieveAll();
}

int main()
{
    printf("main(): pid = %d\n", getpid());

    std::string line;
    for (int i = 33; i < 127; ++i)
    {
        line.push_back(char(i));
    }
    line += line;

    for (size_t i = 0; i < 127 - 33; ++i)
    {
        message += line.substr(i, 72) + '\n';
    }

    tractor::SockAddr listenAddr(9981);
    tractor::EventLoop loop;

    tractor::TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setWriteCompleteCallback(onWriteComplete);
    server.start();

    loop.loop();
}

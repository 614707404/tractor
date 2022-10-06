#include <tractor/TcpServer.h>
#include <tractor/EventLoop.h>
#include <tractor/SockAddr.h>

#include <stdio.h>

void onConnection(const tractor::TcpConnection::TcpConnectionPtr &conn)
{

    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toString().c_str());
        ::sleep(15);                             // client 在此时意外退出
        conn->send("hello bro, are you alive?"); // server 向已经退出的 client 发送消息， 收到 RST 报文
        conn->send("hello bro, are you alive?"); // server 向发送过 RST 报文的 client 发送消息，触发 SIGPIPE ，导致 server 被关闭

        conn->shutdown();
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

    conn->send(buf->retrieveAsString());
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

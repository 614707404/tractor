#include <tractor/Acceptor.h>
using namespace tractor;
using namespace std;
void newConnection_1(int sockfd, const SockAddr &peerAddr)
{
    printf("newConnection(): accepted a new connection from %s\n",
           peerAddr.toString().c_str());
    ::write(sockfd, "How are you?\n", 13);
    ::close(sockfd);
}
void newConnection_2(int sockfd, const SockAddr &peerAddr)
{
    printf("newConnection(): accepted a new connection from %s\n",
           peerAddr.toString().c_str());
    ::write(sockfd, "halll?\n", 13);
    ::close(sockfd);
}
int main()
{
    printf("main() pid = %d\n", getpid());
    EventLoop loop;
    SockAddr sockaddr_1(9981);
    Acceptor acceptor_1(&loop, sockaddr_1);
    acceptor_1.setNewConnectionCallback(newConnection_1);
    acceptor_1.Listen();
    SockAddr sockaddr_2(9982);
    Acceptor acceptor_2(&loop, sockaddr_2);
    acceptor_2.setNewConnectionCallback(newConnection_2);
    acceptor_2.Listen();
    loop.loop();
}
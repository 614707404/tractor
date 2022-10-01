
#include <tractor/Socket.h>
using namespace tractor;
int main()
{
    SockAddr addr("100.0.0.1", 1234);
    Socket socket;
    socket.Bind(addr);
    socket.Listen();

    return 0;
}
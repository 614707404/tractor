
#include <tractor/Socket.h>
#include <iostream>
using namespace std;
using namespace tractor;
int main()
{
    SockAddr addr(8888);
    cout << addr.toString() << endl;
    Socket socket;
    socket.Bind(addr);
    socket.Listen();
    return 0;
}
#include <tractor/HttpResponse.h>

#include <fstream>
#include <sstream>
using namespace std;
using namespace tractor;

void HttpResponse::appendToBuffer(Buffer *output) const
{
    char buf[32];
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    output->append(statusMessage_);
    output->append("\r\n");

    if (closeConnection_)
    {
        output->append("Connection: close\r\n");
    }
    else
    {
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
        output->append(buf);
        output->append("Connection: Keep-Alive\r\n");
    }

    for (const auto &header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n");
    output->append(body_);
}
void HttpResponse::setBodyByFile(const std::string &url)
{
    ifstream send_file;
    send_file.open("/tractor/resources" + url + ".html");
    if (!send_file.is_open())
    {
        statusCode_ = k404NotFound;
        statusMessage_ = "Not Found";
        closeConnection_ = true;
    }
    ostringstream ss;
    char cc;
    while (send_file.get(cc))
    {
        ss.put(cc);
    }
    body_ = ss.str();
}
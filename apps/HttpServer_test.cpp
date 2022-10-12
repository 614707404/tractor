#include <tractor/EventLoop.h>
#include <tractor/HttpServer.h>
#include <tractor/HttpRequest.h>
#include <tractor/HttpResponse.h>

#include <iostream>
#include <map>

#include <spdlog/spdlog.h>

using namespace std;
using namespace tractor;
bool benchmark = false;

void onRequest(const HttpRequest &req, HttpResponse *resp)
{
    spdlog::info("Headers {} {}.", req.getMethodString(), req.getPath());

    if (!benchmark)
    {
        const std::map<string, string> &headers = req.headers();
        for (const auto &header : headers)
        {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }

    if (req.getPath() == "/")
    {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "Muduo");
        resp->setBody("<html><head><title>This is title</title></head>"
                      "<body><h1>Hello</h1></body></html>");
    }
    else if (req.getPath() == "/hello")
    {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/plain");
        resp->addHeader("Server", "Muduo");
        resp->setBody("hello, world!\n");
    }
    else
    {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
}

int main(int argc, char *argv[])
{
    int numThreads = 0;
    if (argc > 1)
    {
        benchmark = true;

        numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    tractor::SockAddr listenAddr(8000);
    HttpServer server(&loop, listenAddr);
    server.setHttpCallback(onRequest);
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();
}
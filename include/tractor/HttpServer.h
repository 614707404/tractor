#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include <functional>
#include <string>

#include <tractor/HttpRequest.h>
#include <tractor/HttpResponse.h>
#include <tractor/TcpServer.h>

namespace tractor
{
    class HttpServer
    {
    public:
        typedef std::function<void(const HttpRequest &, HttpResponse *)> HttpCallback;
        HttpServer(EventLoop *loop,
                   SockAddr &listenAddr);

        EventLoop *getLoop() const { return server_.getLoop(); }

        /// Not thread safe, callback be registered before calling start().
        void setHttpCallback(const HttpCallback &cb)
        {
            httpCallback_ = cb;
        }

        void setThreadNum(int numThreads)
        {
            server_.setThreadNum(numThreads);
        }

        void start();

    private:
        void onConnection(const TcpServer::TcpConnectionPtr &conn);
        void onMessage(const TcpServer::TcpConnectionPtr &conn,
                       Buffer *buf,
                       int64_t receiveTime);
        void onRequest(const TcpServer::TcpConnectionPtr &, const HttpRequest &);

        TcpServer server_;
        HttpCallback httpCallback_;
    };
}
#endif
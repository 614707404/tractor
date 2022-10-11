#include <tractor/HttpServer.h>
#include <tractor/HttpParser.h>
#include <spdlog/spdlog.h>
#include <functional>
using namespace tractor;

void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}
HttpServer::HttpServer(EventLoop *loop,
                       SockAddr &listenAddr)
    : server_(loop, listenAddr),
      httpCallback_(defaultHttpCallback)
{
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
void HttpServer::start()
{
    spdlog::info("HttpServer starts listening");
    server_.start();
}
void HttpServer::onConnection(const TcpServer::TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        conn->setContext(HttpParser());
    }
}
void HttpServer::onMessage(const TcpServer::TcpConnectionPtr &conn,
                           Buffer *buf,
                           int64_t receiveTime)
{
    HttpParser *context = boost::any_cast<HttpParser>(conn->getContext());
    if (!context->parseRuquest(buf, receiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->gotAll())
    {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpServer::TcpConnectionPtr &conn, const HttpRequest &req)
{
    const std::string &connection = req.getHeader("Connection");
    bool close = connection == "close" ||
                 (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}
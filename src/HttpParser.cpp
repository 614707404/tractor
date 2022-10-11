#include <tractor/HttpParser.h>

#include <algorithm>

using namespace tractor;
bool HttpParser::parseRuquest(Buffer *buf, int64_t receiveTime)
{
    bool ok = true;
    bool hasMore = true;
    while (hasMore)
    {
        if (state_ == kExpectRequestLine)
        {
            // 第一个crlf之前的是请求行
            const char *crlf = buf->findCRLF(); // CRLF, CR \r, LF \n
            if (crlf)
            {
                // 解析请求行
                ok = processRequestLine(buf->peek(), crlf);
                if (ok)
                {
                    httpRequest_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2);
                    state_ = kExpectHeaders; // 进入解析请求头状态
                }
                else
                {
                    hasMore = false; // 解析请求行错误
                }
            }
            else
            {
                hasMore = false;
            }
        }
        // 解析请求头
        else if (state_ == kExpectHeaders)
        {
            const char *crlf = buf->findCRLF();
            if (crlf)
            {
                const char *colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf)
                {
                    httpRequest_.addHeader(buf->peek(), colon, crlf);
                }
                else
                {
                    state_ = kGotAll;
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2);
            }
            else
            {
                hasMore = false;
            }
        }
        else if (state_ == kExpectBody)
        {
            // TODO
        }
    }
    return ok;
}
bool HttpParser::processRequestLine(const char *begin, const char *end)
{
    bool ret = false;
    const char *start = begin;
    const char *space = std::find(start, end, ' ');
    if (space != end && httpRequest_.setMethod(start, space))
    {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end)
        {
            const char *question = std::find(start, space, '?');
            if (question != space)
            {
                httpRequest_.setPath(start, question);
                httpRequest_.setQuery(question, space);
            }
            else
            {
                httpRequest_.setPath(start, space);
            }
            start = space + 1;
            ret = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (ret)
            {
                if (*(end - 1) == '1')
                {
                    httpRequest_.setVersion(HttpRequest::kHttp11);
                }
                else if (*(end - 1) == '0')
                {
                    httpRequest_.setVersion(HttpRequest::kHttp10);
                }
                else
                {
                    ret = false;
                }
            }
        }
    }
    return ret;
}
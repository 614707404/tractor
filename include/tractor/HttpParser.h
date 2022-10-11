#ifndef HTTPPARSER_H
#define HTTPPARSER_H
#include <tractor/HttpRequest.h>
#include <tractor/Buffer.h>
namespace tractor
{
    class HttpParser
    {
    public:
        enum HttpRequestParseState
        {
            kExpectRequestLine,
            kExpectHeaders,
            kExpectBody,
            kGotAll,
        };
        HttpParser() : state_(kExpectRequestLine) {}
        const HttpRequest &request() const
        {
            return httpRequest_;
        }
        bool gotAll() const
        {
            return state_ == kGotAll;
        }
        void reset()
        {
            state_ = kExpectRequestLine;
            HttpRequest dummy;
            httpRequest_.swap(dummy);
        }
        bool parseRuquest(Buffer *, int64_t);

    private:
        bool processRequestLine(const char *begin, const char *end);

        HttpRequestParseState state_;
        HttpRequest httpRequest_;
    };
}
#endif
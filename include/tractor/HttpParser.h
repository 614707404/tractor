#ifndef HTTPPARSER_H
#define HTTPPARSER_H
#include <tractor/HttpRequest.h>
namespace tractor
{
    class HttpParser
    {
    public:
        enum CHECK_STATE
        {
            CHECK_STATE_REQUESTLINE = 0,
            CHECK_STATE_HEADER,
            CHECK_STATE_CONTENT
        };
        enum LINE_STATUS
        {
            LINE_OK = 0,
            LINE_BAD,
            LINE_OPEN
        };
        enum HTTP_CODE
        {
            NO_REQUEST,
            GET_REQUEST,
            BAD_REQUEST,
            FORBIDDEN_REQUEST,
            INTERNAL_ERROR,
            CLOSED_CONNECTION
        };

    private:
        HttpRequest httpRequest_;
    };
}
#endif
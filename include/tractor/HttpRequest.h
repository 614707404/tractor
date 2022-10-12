#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <assert.h>
#include <string>
#include <map>
namespace tractor
{
    class HttpRequest
    {
    public:
        enum Method
        {
            kInvalid,
            kGet,     // 请求获取由Request-URI所标识的资源。
            kPost,    // 在Request-URI所标识的资源后附加新的数据
            kHead,    // 请求获取由Request-URI所标识的资源的响应消息报头。
            kPut,     // 请求服务器存储一个资源，并用Request-URI作为其标识。
            kDelete,  // 请求服务器删除由Request-URI所标识的资源
            kTrace,   // 请求服务器回送收到的请求信息，主要用语测试或诊断。
            kOptions, // 请求查询服务器的性能，或查询与资源相关的选项和需求。
        };
        enum Version
        {
            kUnknown,
            kHttp10,
            kHttp11
        };
        HttpRequest()
            : method_(kInvalid),
              version_(kUnknown) {}

        void setVersion(Version v) { version_ = v; }
        Version getVersion() const { return version_; }

        bool setMethod(const char *start, const char *end)
        {
            assert(method_ == kInvalid);
            std::string m(start, end);
            if (m == "GET")
            {
                method_ = kGet;
            }
            else if (m == "POST")
            {
                method_ = kPost;
            }
            else if (m == "HEAD")
            {
                method_ = kHead;
            }
            else if (m == "PUT")
            {
                method_ = kPut;
            }
            else if (m == "DELETE")
            {
                method_ = kDelete;
            }
            else if (m == "TRACE")
            {
                method_ = kTrace;
            }
            else if (m == "OPTIONS")
            {
                method_ = kOptions;
            }
            else
            {
                method_ = kInvalid;
            }
            return method_ != kInvalid;
        }

        Method getMethod() const { return method_; }
        std::string getMethodString() const
        {
            const char *result = "UNKNOWN";
            switch (method_)
            {
            case kGet:
                result = "GET";
                break;
            case kPost:
                result = "POST";
                break;
            case kHead:
                result = "HEAD";
                break;
            case kPut:
                result = "PUT";
                break;
            case kDelete:
                result = "DELETE";
                break;
            default:
                break;
            }
            return result;
        }
        void setPath(const char *start, const char *end)
        {
            path_.assign(start, end);
        }

        const std::string &getPath() const
        {
            return path_;
        }

        void setQuery(const char *start, const char *end)
        {
            query_.assign(start, end);
        }

        const std::string &getQuery() const
        {
            return query_;
        }

        void setReceiveTime(int64_t t)
        {
            receiveTime_ = t;
        }

        int64_t getReceiveTime() const
        {
            return receiveTime_;
        }
        void addHeader(const char *start, const char *colon, const char *end)
        {
            std::string field(start, colon);
            ++colon;
            // 去空格
            while (colon < end && isspace(*colon))
            {
                ++colon;
            }
            std::string value(colon, end);
            // 去空格
            while (!value.empty() && isspace(value[value.size() - 1]))
            {
                value.resize(value.size() - 1);
            }
            headers_[field] = value;
        }

        std::string getHeader(const std::string &field) const
        {
            std::string result;
            std::map<std::string, std::string>::const_iterator it = headers_.find(field);
            if (it != headers_.end())
            {
                result = it->second;
            }
            return result;
        }

        const std::map<std::string, std::string> &headers() const
        {
            return headers_;
        }

        void swap(HttpRequest &that)
        {
            std::swap(method_, that.method_);
            std::swap(version_, that.version_);
            path_.swap(that.path_);
            query_.swap(that.query_);
            receiveTime_ = that.receiveTime_;
            headers_.swap(that.headers_);
        }

    private:
        Version version_;
        Method method_;
        std::string path_;
        std::string query_;
        int64_t receiveTime_;
        std::map<std::string, std::string> headers_;
    };
} // namespace tractor

#endif
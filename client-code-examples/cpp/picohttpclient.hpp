/**
 * Modified by kyuridenamida to enable POST sending.
 * Copied from https://github.com/csachs/picohttpclient
 * Huge thanks for https://github.com/csachs!
 */

/*
  picohttpclient.hpp ... generic, lightweight HTTP 1.1 client
  ... no complex features, no chunking, no ssl, no keepalive ...
  ... not very tested, use at your own risk!
  ... it PURPOSELY does not use any feature-complete libraries
      (like cURL) to stay lean and header-only.
  ... it does not use C++11 features to fit well in legacy code bases
  ... it has some suboptimal properties (like many string copy ops)
  The MIT License
  Copyright (c) 2016 Christian C. Sachs
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// possibly add SSL?
// https://wiki.openssl.org/index.php/SSL/TLS_Client

namespace PicoHTTPClient {
    using namespace std;

    class tokenizer {
    public:
        inline tokenizer(string &str) : str(str), position(0) {};

        inline string next(string search, bool returnTail = false) {
            size_t hit = str.find(search, position);
            if (hit == string::npos) {
                if (returnTail) {
                    return tail();
                } else {
                    return "";
                }
            }

            size_t oldPosition = position;
            position = hit + search.length();

            return str.substr(oldPosition, hit - oldPosition);
        };

        inline string tail() {
            size_t oldPosition = position;
            position = str.length();
            return str.substr(oldPosition);
        };

    private:
        string str;
        size_t position;
    };

    typedef map <string, string> stringMap;

    struct URI {
        inline void parseParameters() {
            tokenizer qt(querystring);
            do {
                string key = qt.next("=");
                if (key == "")
                    break;
                parameters[key] = qt.next("&", true);
            } while (true);
        }

        inline URI(string input, bool shouldParseParameters = false) {
            tokenizer t = tokenizer(input);
            protocol = t.next("://");
            string hostPortString = t.next("/");

            tokenizer hostPort(hostPortString);

            host = hostPort.next(hostPortString[0] == '[' ? "]:" : ":", true);

            if (host[0] == '[')
                host = host.substr(1, host.size() - 1);

            port = hostPort.tail();

            address = t.next("?", true);
            querystring = t.next("#", true);

            hash = t.tail();

            if (shouldParseParameters) {
                parseParameters();
            }
        };

        string protocol, host, port, address, querystring, hash;
        stringMap parameters;
    };

    struct HTTPResponse {
        bool success;
        string protocol;
        string response;
        string responseString;

        stringMap header;

        string body;

        inline HTTPResponse() : success(true) {};

        inline static HTTPResponse fail() {
            HTTPResponse result;
            result.success = false;
            return result;
        }
    };

    struct HTTPClient {
        typedef enum {
            OPTIONS = 0,
            GET,
            HEAD,
            POST,
            PUT,
            DELETE,
            TRACE,
            CONNECT
        } HTTPMethod;

        inline static const char *method2string(HTTPMethod method) {
            const char *methods[] = {"OPTIONS", "GET", "HEAD", "POST", "PUT",
                                     "DELETE", "TRACE", "CONNECT", NULL};
            return methods[method];
        };

        inline static int connectToURI(URI uri) {
            struct addrinfo hints, *result, *rp;

            memset(&hints, 0, sizeof(addrinfo));

            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            if (uri.port == "") {
                uri.port = "80";
            }

            int getaddrinfo_result =
                    getaddrinfo(uri.host.c_str(), uri.port.c_str(), &hints, &result);

            if (getaddrinfo_result != 0)
                return -1;

            int fd = -1;

            for (rp = result; rp != NULL; rp = rp->ai_next) {

                fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

                if (fd == -1) {
                    continue;
                }

                int connect_result = connect(fd, rp->ai_addr, rp->ai_addrlen);

                if (connect_result == -1) {
                    // successfully created a socket, but connection failed. close it!
                    close(fd);
                    fd = -1;
                    continue;
                }

                break;
            }

            freeaddrinfo(result);

            return fd;
        };

        inline static string bufferedRead(int fd) {
            size_t initial_factor = 4, buffer_increment_size = 8192, buffer_size = 0,
                    bytes_read = 0;
            string buffer;

            buffer.resize(initial_factor * buffer_increment_size);

            do {
                bytes_read = read(fd, ((char *) buffer.c_str()) + buffer_size,
                                  buffer.size() - buffer_size);

                buffer_size += bytes_read;

                if (bytes_read > 0 &&
                    (buffer.size() - buffer_size) < buffer_increment_size) {
                    buffer.resize(buffer.size() + buffer_increment_size);
                }
            } while (bytes_read > 0);

            buffer.resize(buffer_size);
            return buffer;
        };


        inline static HTTPResponse request(HTTPMethod method, URI uri, string bodyJson) {
#define HTTP_NEWLINE "\r\n"
#define HTTP_SPACE " "
#define HTTP_HEADER_SEPARATOR ": "

            int fd = connectToURI(uri);
            if (fd < 0)
                return HTTPResponse::fail();

            string request = string(method2string(method)) + string(" /") +
                             uri.address + ((uri.querystring == "") ? "" : "?") +
                             uri.querystring + " HTTP/1.1" HTTP_NEWLINE "Host: " +
                             uri.host + HTTP_NEWLINE
                                        "Accept: */*" HTTP_NEWLINE
                                        "Connection: close" HTTP_NEWLINE
                                        "Content-Length: " + itos(bodyJson.size()) + HTTP_NEWLINE
                                                                                     "Content-Type: application/json" HTTP_NEWLINE HTTP_NEWLINE;
            request += bodyJson;
            int bytes_written = write(fd, request.c_str(), request.size());
            string buffer = bufferedRead(fd);

            close(fd);

            HTTPResponse result;

            tokenizer bt(buffer);

            result.protocol = bt.next(HTTP_SPACE);
            result.response = bt.next(HTTP_SPACE);
            result.responseString = bt.next(HTTP_NEWLINE);

            string header = bt.next(HTTP_NEWLINE HTTP_NEWLINE);

            result.body = bt.tail();

            tokenizer ht(header);

            do {
                string key = ht.next(HTTP_HEADER_SEPARATOR);
                if (key == "")
                    break;
                result.header[key] = ht.next(HTTP_NEWLINE, true);
            } while (true);

            return result;
        };
    private:

        static string itos(int n) {
            stringstream ss;
            ss << n;
            return ss.str();
        }
    };
};
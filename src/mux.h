#pragma once

#include <iostream>

#include "Request.h"
#include "ResponseWriter.h"

void mux(Request request, ResponseWriter response_writer)
{
    std::cout << "MUX:::" << request.method << std::endl;

    std::string body = R"(<html>
<body>
<h1>Hello, World!</h1>
</body>
</html>)";

    std::string text = R"|(HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache/2.2.14 (Win32)
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
Content-Length: )|" + std::to_string(body.size()) + R"|(
Content-Type: text/html
Connection: keep-alive

)|" + body;

    response_writer.write_text(text);
}
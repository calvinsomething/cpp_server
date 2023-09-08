#include "mux.h"

#include <iostream>

void mux(Request request, ResponseWriter response_writer)
{
    std::string body = R"(<html>
<body>
<h1>Hello, World!</h1>
</body>
</html>)";

    response_writer.send_text(200, body);
}

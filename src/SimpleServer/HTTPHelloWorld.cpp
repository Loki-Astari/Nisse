#include "HTTPHelloWorld.h"
#include "HTTP/Response.h"
#include <string_view>

void HTTPHelloWorld::processRequest(ThorsAnvil::Nisse::HTTP::Request const& request, ThorsAnvil::Nisse::HTTP::Response& response)
{
    std::string_view page = R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world</body>
</html>
)";

    ThorsAnvil::Nisse::HTTP::HeaderResponse   header;
    response.addHeaders(header, page.size()) << page;
}

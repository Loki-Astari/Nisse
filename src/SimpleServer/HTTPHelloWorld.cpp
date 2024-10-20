#include "HTTPHelloWorld.h"
#include "NisseHTTP/Response.h"
#include <string_view>

void HTTPHelloWorld::processRequest(ThorsAnvil::Nisse::NisseHTTP::Request const& request, ThorsAnvil::Nisse::NisseHTTP::Response& response)
{
    std::string_view page = R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world</body>
</html>
)";

    ThorsAnvil::Nisse::NisseHTTP::HeaderResponse   header;
    response.addHeaders(header, page.size()) << page;
}

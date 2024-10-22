#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"

inline
void addRest(ThorsAnvil::Nisse::NisseHTTP::HTTPHandler& http)
{
    http.addPath("/HW{Who}.html", [](ThorsAnvil::Nisse::NisseHTTP::Request& request, ThorsAnvil::Nisse::NisseHTTP::Response& response)
    {
        std::string who  = request.variables()["Who"];
        std::string data = R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )"  + who + R"(</body>
</html>
)";
        ThorsAnvil::Nisse::NisseHTTP::HeaderResponse   header;
        response.addHeaders(header, data.size()) << data;
    });
    http.addPath("/CK{Who}.html", [](ThorsAnvil::Nisse::NisseHTTP::Request& request, ThorsAnvil::Nisse::NisseHTTP::Response& response)
    {
        ThorsAnvil::Nisse::NisseHTTP::HeaderResponse   header;
        response.addHeaders(header, ThorsAnvil::Nisse::NisseHTTP::Encoding::Chunked) << R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )" << request.variables()["Who"] << R"(</body>
</html>
)";
    });
}

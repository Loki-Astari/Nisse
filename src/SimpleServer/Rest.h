#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"

namespace NHTTP     = ThorsAnvil::Nisse::HTTP;

inline
void addRest(NHTTP::HTTPHandler& http)
{
    http.addPath("/HW{Who}.html", [](NHTTP::Request& request, NHTTP::Response& response)
    {
        std::string who  = request.variables()["Who"];
        std::string data = R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )"  + who + R"(</body>
</html>
)";
        NHTTP::HeaderResponse   header;
        response.addHeaders(header);
        response.body(data.size()) << data;
    });
    http.addPath("/CK{Who}.html", [](NHTTP::Request& request, NHTTP::Response& response)
    {
        NHTTP::HeaderResponse   header;
        response.addHeaders(header);
        response.body(NHTTP::Encoding::Chunked) << R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )" << request.variables()["Who"] << R"(</body>
</html>
)";
    });
}

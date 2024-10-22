#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include "NisseServer/AsyncStream.h"
#include <string>

inline
std::string normalize(std::string const& path, std::string const& fileName)
{
    return path + "/" + fileName;
}

inline
void addFiles(ThorsAnvil::Nisse::NisseHTTP::HTTPHandler& http)
{
    http.addPath("/content/{file}", [](ThorsAnvil::Nisse::NisseHTTP::Request& request, ThorsAnvil::Nisse::NisseHTTP::Response& response)
    {
        ThorsAnvil::Nisse::NisseHTTP::HeaderResponse    header;

        using std::literals::string_literals::operator""s;

        std::string                     fileName = normalize("/Users/martinyork/Repo/Nisse/src/SimpleServer/content"s, request.variables()["file"]);
        ThorsAnvil::Nisse::IFStream     file(fileName, request.getContext());
        if (!file)
        {
            response.setStatus(404);
            response.addHeaders(header, 0);
        }
        else
        {
            response.addHeaders(header, ThorsAnvil::Nisse::NisseHTTP::Encoding::Chunked)
                << file.rdbuf();
        }
    });
}

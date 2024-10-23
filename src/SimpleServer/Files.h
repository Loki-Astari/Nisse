#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include "NisseServer/AsyncStream.h"
#include <string>

namespace NServer   = ThorsAnvil::Nisse::Server;
namespace NHTTP     = ThorsAnvil::Nisse::HTTP;

inline
std::string normalize(std::string const& path, std::string const& fileName)
{
    return path + "/" + fileName;
}

inline
void addFiles(NHTTP::HTTPHandler& http)
{
    http.addPath("/content/{file}", [](NHTTP::Request& request, NHTTP::Response& response)
    {
        NHTTP::HeaderResponse    header;

        using std::literals::string_literals::operator""s;

        std::string           fileName = normalize("/Users/martinyork/Repo/Nisse/src/SimpleServer/content"s, request.variables()["file"]);
        TAS::SocketStream     file{TAS::Socket{TAS::FileInfo{fileName, TAS::FileMode::Read}, TAS::Blocking::No}};
        NServer::AsyncStream  async(file, request.getContext(), NServer::EventType::Read);

        if (file)
        {
            response.addHeaders(header, NHTTP::Encoding::Chunked)
                << file.rdbuf();
        }
        else
        {
            response.setStatus(404);
            response.addHeaders(header, 0);
        }
    });
}

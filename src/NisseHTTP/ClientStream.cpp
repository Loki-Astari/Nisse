#include "ClientStream.h"

using namespace ThorsAnvil::Nisse::HTTP;

std::string ClientStream::getHost(std::string const& url)
{
    auto find = url.find("://");
    if (find == std::string::npos) {
        find = 0;
    }
    else {
        find += 3;
    }
    auto path = std::min(std::size(url), url.find('/', find));
    auto port = url.find(':', find);
    auto end  = std::min(path, port);
    return url.substr(find, (end - find));
}

int ClientStream::getPort(std::string const& url)
{
    auto find = url.find("://");
    if (find == std::string::npos) {
        find = 0;
    }
    else {
        find += 3;
    }
    auto path = std::min(std::size(url), url.find('/', find));
    auto port = url.find(':', find);
    if (port < path) {
        return std::stoi(url.substr(port));
    }
    return 443;
}

ClientStream::ClientStream(std::string const& url)
    : ctx{ThorsAnvil::ThorsSocket::SSLMethodType::Client}
    , stream{ThorsAnvil::ThorsSocket::SSocketInfo{getHost(url), getPort(url), ctx, ThorsAnvil::ThorsSocket::DeferAccept::No}}
{}

#include "URL.h"

using namespace ThorsAnvil::Nisse::HTTP;

URL::URL(std::string_view href)
    : hrefValue{href}
    , protocolRef{findProtocol(hrefValue)}
    , originRef{findOrigin(hrefValue)}
    , hostRef{findHost(hrefValue)}
    , portRef{findPort(hrefValue)}
    , hostnameRef{findHostname(hrefValue)}
    , pathRef{findPath(hrefValue)}
    , queryRef{findQuery(hrefValue)}
    , hashRef{findHash(hrefValue)}
{}

URL::URL(std::string_view prot, std::string_view host, std::string_view request)
    : hrefValue{buildHref(prot, host, request)}
    , protocolRef{findProtocol(hrefValue)}
    , originRef{findOrigin(hrefValue)}
    , hostRef{findHost(hrefValue)}
    , portRef{findPort(hrefValue)}
    , hostnameRef{findHostname(hrefValue)}
    , pathRef{findPath(hrefValue)}
    , queryRef{findQuery(hrefValue)}
    , hashRef{findHash(hrefValue)}
{}

URL::URL(URL const& copy)
    : hrefValue{copy.hrefValue}
    , protocolRef{findProtocol(hrefValue)}
    , originRef{findOrigin(hrefValue)}
    , hostRef{findHost(hrefValue)}
    , portRef{findPort(hrefValue)}
    , hostnameRef{findHostname(hrefValue)}
    , pathRef{findPath(hrefValue)}
    , queryRef{findQuery(hrefValue)}
    , hashRef{findHash(hrefValue)}
{}

URL::URL(URL&& move) noexcept
    // All members default initialized to empty.
{
    swap(move);
}

URL& URL::operator=(URL copyORmove) noexcept
{
    swap(copyORmove);
    return *this;
}

void URL::swap(URL& other) noexcept
{
    using std::swap;
    swap(hrefValue,     other.hrefValue);
    swap(protocolRef,   other.protocolRef);
    swap(originRef,     other.originRef);
    swap(hostRef,       other.hostRef);
    swap(portRef,       other.portRef);
    swap(hostnameRef,   other.hostnameRef);
    swap(pathRef,       other.pathRef);
    swap(queryRef,      other.queryRef);
    swap(hashRef,       other.hashRef);
}

std::string URL::buildHref(std::string_view prot, std::string_view host, std::string_view request)
{
    std::string href;

    href = prot;
    href += "://";
    href += host;
    href += request;

    return href;
}

std::string_view URL::findProtocol(std::string const& src)
{
    std::size_t size = std::min(src.size(), src.find("://"));
    return {src.begin(), src.begin() + size};
}

std::string_view URL::findOrigin(std::string const& src)
{
    std::size_t skipProto = std::min(src.size(), protocolRef.size() + 3);
    std::size_t size = std::min(src.size(), src.find_first_of("/?&#", skipProto));
    return {src.begin(), src.begin() + size};
}

std::string_view URL::findHost(std::string const& src)
{
    std::size_t beg = std::min(src.size(), protocolRef.size() + 3);
    std::size_t end = std::min(src.size(), originRef.size());
    return {src.begin() + beg, src.begin() + end};
}

std::string_view URL::findPort(std::string const&)
{
    std::string_view result = hostRef;
    std::size_t find = std::min(result.size(), result.find(':'));
    result.remove_prefix(find);
    return result;
}

std::string_view URL::findHostname(std::string const&)
{
    std::string_view result = hostRef;
    result.remove_suffix(portRef.size());
    return result;
}

std::string_view URL::findPath(std::string const& src)
{
    std::size_t skipProto = std::min(src.size(), protocolRef.size() + 3);
    std::size_t beg = std::min(src.size(), src.find_first_of("/?&#", skipProto));
    std::size_t end = std::min(src.size(), src.find_first_of("?&#", beg));
    return {src.begin() + beg, src.begin() + end};
}

std::string_view URL::findQuery(std::string const& src)
{
    std::size_t skipProto = std::min(src.size(), protocolRef.size() + 3);
    std::size_t beg = std::min(src.size(), src.find_first_of("?&#", skipProto));
    std::size_t end = std::min(src.size(), src.find_first_of("#", beg));
    return {src.begin() + beg, src.begin() + end};
}

std::string_view URL::findHash(std::string const& src)
{
    std::size_t skipProto = std::min(src.size(), protocolRef.size() + 3);
    std::size_t beg = std::min(src.size(), src.find_first_of("#", skipProto));
    std::size_t end = src.size();
    return {src.begin() + beg, src.begin() + end};
}

std::string_view URL::param(std::string_view /*param*/)
{
    // TODO
    return "";
}

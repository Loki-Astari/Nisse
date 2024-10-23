#include "AsyncStream.h"

using namespace ThorsAnvil::Nisse::Server;

IFStream::IFStream(std::string filename)
    : TAS::SocketStream{TAS::Socket{TAS::FileInfo{filename, TAS::FileMode::Read}, TAS::Blocking::No}}
    , registeredContext{nullptr}
{}

IFStream::IFStream(std::string filename, Context& context)
    : TAS::SocketStream{TAS::Socket{TAS::FileInfo{filename, TAS::FileMode::Read}, TAS::Blocking::No}}
    , registeredContext{&context}
{
    if (*this) {
        registeredContext->registerLocalSocket(getSocket(), EventType::Read);
    }
    else {
        registeredContext = nullptr;
    }
}

IFStream::~IFStream()
{
    if (registeredContext) {
        registeredContext->unregisterLocalSocket(getSocket());
    }
}

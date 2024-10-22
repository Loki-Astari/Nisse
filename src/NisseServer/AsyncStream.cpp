#include "AsyncStream.h"

using namespace ThorsAnvil::Nisse;

IFStream::IFStream(std::string filename)
    : TAS::SocketStream{TAS::Socket{TAS::FileInfo{filename, TAS::Open::Append}, TAS::Blocking::No}}
    , registeredContext{nullptr}
{}

IFStream::IFStream(std::string filename, Context& context)
    : TAS::SocketStream{TAS::Socket{TAS::FileInfo{filename, TAS::Open::Append}, TAS::Blocking::No}}
    , registeredContext{&context}
{
    registeredContext->registerLocalSocket(getSocket(), EventType::Read);
}

IFStream::~IFStream()
{
    if (registeredContext) {
        registeredContext->unregisterLocalSocket(getSocket());
    }
}

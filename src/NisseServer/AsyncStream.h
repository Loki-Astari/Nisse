#ifndef THORSANVIL_NISSE_ASYNC_STREAM_H
#define THORSANVIL_NISSE_ASYNC_STREAM_H

#include "NisseConfig.h"
#include "Context.h"
#include "ThorsSocket/Socket.h"
#include "ThorsSocket/SocketStream.h"

namespace TAS = ThorsAnvil::ThorsSocket;
namespace ThorsAnvil::Nisse
{

class IFStream: public TAS::SocketStream
{
    Context*    registeredContext;
    public:
        IFStream(std::string filename);
        IFStream(std::string filename, Context& context);
        ~IFStream();
};

}

#endif

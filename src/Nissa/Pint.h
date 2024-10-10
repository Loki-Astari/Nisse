#ifndef THORSANVIL_NISSA_PINT_H
#define THORSANVIL_NISSA_PINT_H

#include "NissaConfig.h"
#include "EventHandler.h"
#include "ThorsSocket/SocketStream.h"
#include <functional>

namespace ThorsAnvil::Nissa
{

class Pint
{
    public:
        using SocketStream = ThorsAnvil::ThorsSocket::SocketStream;
        using ServerAction = std::function<bool(SocketStream&&)>;

        virtual ~Pint() {}
        virtual EventTask handleRequest(SocketStream& stream)    = 0;
};

}

#endif

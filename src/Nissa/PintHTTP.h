#ifndef THORSANVIL_NISSA_PINT_HTTP_H
#define THORSANVIL_NISSA_PINT_HTTP_H

#include "NissaConfig.h"
#include "Pint.h"

namespace ThorsAnvil::Nissa
{

class PintHTTP: public Pint
{
    using SocketStream = Pint::SocketStream;
    using ServerAction = Pint::ServerAction;

    public:
        virtual PintResult handleRequest(SocketStream& stream) override;
};

}

#endif

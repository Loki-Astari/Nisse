#ifndef THORSANVIL_NISSE_PYNT_H
#define THORSANVIL_NISSE_PYNT_H

/*
 * Virtual base class for representing different protocols.
 *
 * You can add a Pynt to a port on a Nisse server.
 * Any connections that come to this port will result in a call to the 'handleRequest' method.
 *
 * The 'handleRequest' has a single parameter 'stream'.
 *  Reading from stream reads from the socket.
 *  Writing to the stream writes to the socket.
 *
 * Note: The stream is non blocking.
 *       If a read/write would block the thread will pause your code
 *       do something else and return to your code when the stream is OK to use again.
 *       Your code does not have to do anything special to handle this simply write the
 *       code as if the stream were blocking.
 *
 * Return:  PyntResult::Done    Indicates all communication is done and the server should close the socket.
 *          PyntResult::More    Indicates this request has completed but there may be another.
 *                              So keep the socket open to see if the client makes another request.
 *                              Example: HTTP with header 'Connection: keep-alive'
 */

#include "NisseConfig.h"
#include "Context.h"
#include <ThorsSocket/SocketStream.h>

namespace TAS = ThorsAnvil::ThorsSocket;
namespace ThorsAnvil::Nisse
{

enum class PyntResult {Done, More};

class Pynt
{
    public:
        virtual ~Pynt();
        virtual PyntResult handleRequest(TAS::SocketStream& stream, Context& context) = 0;
};

}

#endif

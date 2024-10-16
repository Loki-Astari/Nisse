#include "PyntControl.h"
#include <charconv>

namespace TAS   = ThorsAnvil::ThorsSocket;
using namespace ThorsAnvil::Nisse;

PyntControl::PyntControl(NisseServer& server)
    : server(server)
{}

PyntResult PyntControl::handleRequest(TAS::SocketStream& stream)
{
    server.stop();
    return PyntResult::Done;
}

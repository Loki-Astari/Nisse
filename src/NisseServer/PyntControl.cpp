#include "PyntControl.h"

namespace TASock = ThorsAnvil::ThorsSocket;

using namespace ThorsAnvil::Nisse::Server;

PyntControl::PyntControl(NisseServer& server)
    : server{server}
{}

PyntResult PyntControl::handleRequest(TASock::SocketStream& /*stream*/, Context&)
{
    server.stop();
    return PyntResult::Done;
}

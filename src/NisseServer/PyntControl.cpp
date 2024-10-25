#include "PyntControl.h"

using namespace ThorsAnvil::Nisse::Server;

PyntControl::PyntControl(NisseServer& server)
    : server{server}
{}

PyntResult PyntControl::handleRequest(TAS::SocketStream& /*stream*/, Context&)
{
    server.stop();
    return PyntResult::Done;
}

#include "PyntControl.h"

using namespace ThorsAnvil::Nisse;

PyntControl::PyntControl(NisseServer& server)
    : server{server}
{}

PyntResult PyntControl::handleRequest(TAS::SocketStream& /*stream*/)
{
    server.stop();
    return PyntResult::Done;
}

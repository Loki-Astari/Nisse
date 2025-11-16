#include "PyntControl.h"

namespace TASock = ThorsAnvil::ThorsSocket;

using namespace ThorsAnvil::Nisse::Server;

PyntControl::PyntControl(NisseServer& server)
    : server{server}
{}

PyntResult PyntControl::handleRequest(TASock::SocketStream& /*stream*/, Context&)
{
    // This is a very simple control.
    // Please checkout HTTPPyntControl for a slightly more sophisticated option.
    server.stopHard();
    return PyntResult::Done;
}

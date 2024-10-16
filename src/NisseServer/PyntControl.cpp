#include "PyntControl.h"

using namespace ThorsAnvil::Nisse;

PyntControl::PyntControl(NisseServer& server)
    : server(server)
{}

PyntResult PyntControl::handleRequest(std::iostream& stream)
{
    server.stop();
    return PyntResult::Done;
}

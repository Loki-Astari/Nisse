#include "Util.h"
#include <ostream>

using namespace ThorsAnvil::Nisse::HTTP;

namespace ThorsAnvil::Nisse::HTTP
{
std::ostream& operator<<(std::ostream& stream, Version const& v)
{
    switch (v)
    {
        case Version::HTTP1_0:  stream << "HTTP/1.0";break;
        case Version::HTTP1_1:  stream << "HTTP/1.1";break;
        case Version::HTTP2:    stream << "HTTP/2";break;
        case Version::HTTP3:    stream << "HTTP/3";break;
        case Version::Unknown:  break;
    }
    return stream;
}
}

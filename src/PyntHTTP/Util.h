#ifndef THORSANVIL_NISSE_PYNTHTTP_UTIL_H
#define THORSANVIL_NISSE_PYNTHTTP_UTIL_H

#include "PyntHTTPConfig.h"
#include <set>
#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>

namespace ThorsAnvil::Nisse::PyntHTTP
{

enum class Version {HTTP1_0, HTTP1_1, HTTP2, HTTP3, Unknown};
enum class Method  {GET, HEAD, OPTIONS, TRACE, PUT, DELETE, POST, PATCH, CONNECT, Other};
enum class Encoding{Chunked};

}

#endif

#ifndef THORSANVIL_NISSA_ACTION_H
#define THORSANVIL_NISSA_ACTION_H

#include "NissaConfig.h"
#include <ThorsSocket/SocketStream.h>
#include <boost/coroutine2/all.hpp>

namespace ThorsAnvil::Nissa
{

enum class TaskYieldState        {RestoreRead, RestoreWrite, Remove};
struct ServerData;
struct StreamData;

using CoRoutine     = boost::coroutines2::coroutine<TaskYieldState>::pull_type;
using Yield         = boost::coroutines2::coroutine<TaskYieldState>::push_type;
using ServerTask    = std::function<void(ThorsAnvil::ThorsSocket::Server& stream, Yield& yield)>;
using StreamTask    = std::function<void(ThorsAnvil::ThorsSocket::SocketStream& stream, Yield& yield)>;
using ServerCreator = std::function<CoRoutine(ServerData&)>;
using StreamCreator = std::function<CoRoutine(StreamData&)>;

}

#endif

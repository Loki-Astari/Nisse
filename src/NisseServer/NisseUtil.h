#ifndef THORSANVIL_NISSE_NISSE_UTIL_H
#define THORSANVIL_NISSE_NISSE_UTIL_H

#include "NisseConfig.h"
#include <ThorsSocket/Server.h>
#include <ThorsSocket/SocketStream.h>
#include <boost/coroutine2/all.hpp>
#include <functional>

namespace TAS   = ThorsAnvil::ThorsSocket;
namespace ThorsAnvil::Nisse
{
/*
 * Forward declare Store objects.
 * See Store.h for details
 */
struct ServerData;
struct StreamData;

/*
 * Action to be performed by EventHandler
 * When the work code is blocked.
 * See EventHandler for details.
 */
enum class TaskYieldState        {RestoreRead, RestoreWrite, Remove};

/*
 * The types need to do the work.
 * Created by the Server object.
 */
using CoRoutine     = boost::coroutines2::coroutine<TaskYieldState>::pull_type;
using Yield         = boost::coroutines2::coroutine<TaskYieldState>::push_type;
using ServerTask    = std::function<void(TAS::Server& stream, Yield& yield)>;
using StreamTask    = std::function<void(TAS::SocketStream& stream, Yield& yield)>;
using ServerCreator = std::function<CoRoutine(ServerData&)>;
using StreamCreator = std::function<CoRoutine(StreamData&)>;

}

#endif

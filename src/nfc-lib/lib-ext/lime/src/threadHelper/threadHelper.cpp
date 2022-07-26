#include "threadHelper.h"

#ifdef __unix__
#include <pthread.h>
#else
#include <windows.h>
#endif
#include "Logger.h"

using namespace lime;

#ifdef __unix__
int lime::SetOSThreadPriority(ThreadPriority priority, ThreadPolicy policy, std::thread *thread)
{
    if (!thread)
    {
        lime::debug("SetOSThreadPriority: null thread pointer");
        return -1;
    }

    int sched_policy = SCHED_OTHER;

    switch (policy)
    {
    case ThreadPolicy::DEFAULT:
        sched_policy = SCHED_OTHER;
        break;
    case ThreadPolicy::REALTIME:
        sched_policy = SCHED_RR;
        break;
    case ThreadPolicy::PREEMPTIVE:
        sched_policy = SCHED_FIFO;
        break;
    }

    int prio_min = sched_get_priority_min(sched_policy);
    int prio_max = sched_get_priority_max(sched_policy);

    if (prio_min == -1 || prio_max == -1)
    {
        lime::debug("SetOSThreadPriority: Failed to get available priority values");
        return -1;
    }

    //scale values
    sched_param sch;
    sch.sched_priority = int(((prio_max - prio_min) / (float)ThreadPriority::HIGHEST) * priority) + prio_min;

    if (int ret = pthread_setschedparam(thread->native_handle(), sched_policy, &sch))
    {
        lime::debug("SetOSThreadPriority: Failed to set priority(%d), schec_prio(%d), policy(%d), ret(%d)",
                    priority, sched_policy, ret, sch.sched_priority);
        return -1;
    }

    return 0;
}

int lime::SetOSCurrentThreadPriority(ThreadPriority priority, ThreadPolicy policy)
{
    int sched_policy = SCHED_OTHER;

    switch (policy)
    {
    case ThreadPolicy::DEFAULT:
        sched_policy = SCHED_OTHER;
        break;
    case ThreadPolicy::REALTIME:
        sched_policy = SCHED_RR;
        break;
    case ThreadPolicy::PREEMPTIVE:
        sched_policy = SCHED_FIFO;
        break;
    }

    int prio_min = sched_get_priority_min(sched_policy);
    int prio_max = sched_get_priority_max(sched_policy);

    if (prio_min == -1 || prio_max == -1)
    {
        lime::debug("SetOSCurrentThreadPriority: Failed to get available priority values");
        return -1;
    }

    //scale values
    sched_param sch;
    sch.sched_priority = int(((prio_max - prio_min) / (float)ThreadPriority::HIGHEST) * priority) + prio_min;

    if (int ret = pthread_setschedparam(pthread_self(), sched_policy, &sch) != 0)
    {
        lime::debug("SetOSCurrentThreadPriority: Failed to set priority(%d), schec_prio(%d), policy(%d), ret(%d)",
                    priority, sched_policy, ret, sch.sched_priority);
        return -1;
    }

    return 0;
}

#elif _WIN32

int lime::SetOSThreadPriority(ThreadPriority priority, ThreadPolicy /*policy*/, std::thread *thread)
{
    if (!thread)
    {
        lime::debug("SetOSThreadPriority: null thread pointer");
        return -1;
    }

    int win_priority = THREAD_PRIORITY_NORMAL;

    switch (priority)
    {
    case ThreadPriority::LOWEST:
        win_priority = THREAD_PRIORITY_IDLE;
        break;
    case ThreadPriority::LOW:
        win_priority = THREAD_PRIORITY_LOWEST;
        break;
    case ThreadPriority::BELOW_NORMAL:
        win_priority = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case ThreadPriority::NORMAL:
        win_priority = THREAD_PRIORITY_NORMAL;
        break;
    case ThreadPriority::ABOVE_NORMAL:
        win_priority = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case ThreadPriority::HIGH:
        win_priority = THREAD_PRIORITY_HIGHEST;
        break;
    case ThreadPriority::HIGHEST:
        win_priority = THREAD_PRIORITY_TIME_CRITICAL;
        break;
    }

    if (!SetThreadPriority(thread->native_handle(), win_priority))
    {
        lime::debug("SetThreadPriority: Failed to set priority(%d)", win_priority);
        return -1;
    }
    return 0;
}

int lime::SetOSCurrentThreadPriority(ThreadPriority priority, ThreadPolicy /*policy*/)
{
    int win_priority = THREAD_PRIORITY_NORMAL;

    switch (priority)
    {
    case ThreadPriority::LOWEST:
        win_priority = THREAD_PRIORITY_IDLE;
        break;
    case ThreadPriority::LOW:
        win_priority = THREAD_PRIORITY_LOWEST;
        break;
    case ThreadPriority::BELOW_NORMAL:
        win_priority = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case ThreadPriority::NORMAL:
        win_priority = THREAD_PRIORITY_NORMAL;
        break;
    case ThreadPriority::ABOVE_NORMAL:
        win_priority = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case ThreadPriority::HIGH:
        win_priority = THREAD_PRIORITY_HIGHEST;
        break;
    case ThreadPriority::HIGHEST:
        win_priority = THREAD_PRIORITY_TIME_CRITICAL;
        break;
    }

    if (!SetThreadPriority(GetCurrentThread(), win_priority))
    {
        lime::debug("SetThreadPriority: Failed to set priority(%d)", win_priority);
        return -1;
    }
    return 0;
}
#else

int lime::SetOSThreadPriority(ThreadPriority priority, ThreadPolicy policy, std::thread *thread)
{
    return 0;
}

int lime::SetOSCurrentThreadPriority(ThreadPriority priority, ThreadPolicy policy)
{
    return 0;
}
#endif

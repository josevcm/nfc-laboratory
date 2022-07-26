#ifndef LIMESUITE_THREAD_H
#define LIMESUITE_THREAD_H

#include <thread>

namespace lime{

enum ThreadPriority {
    LOWEST,
    LOW,
    BELOW_NORMAL,
    NORMAL,
    ABOVE_NORMAL,
    HIGH,
    HIGHEST
};

enum ThreadPolicy {
    DEFAULT,
    REALTIME,
    PREEMPTIVE, // FIFO
};
/**
 * Set priority of current or specificied thread
 *
 * @param priority  Thread priority
 * @param policy    Thread scheduling policy, not used on Windows
 * @param thread    Thread to which set the priority to
 *
 * @return          0 on success, (-1) on failure
 */
int SetOSThreadPriority(ThreadPriority priority, ThreadPolicy policy, std::thread *thread);

/**
 * Set priority of current or specificied thread
 * @note On Windows systems, policy will be used to set either IDLE or TIME_CRITICAL thread priority
 *
 * @param priority  Thread priority
 * @param policy    Thread scheduling policy, not used on Windows
 *
 * @return          0 on success, (-1) on failure
 */
int SetOSCurrentThreadPriority(ThreadPriority priority, ThreadPolicy policy);
}

#endif

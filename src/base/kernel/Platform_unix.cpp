#ifdef XMRIG_OS_FREEBSD
#   include <sys/types.h>
#   include <sys/param.h>
#   ifndef __DragonFly__
#       include <sys/cpuset.h>
#   endif
#   include <pthread_np.h>
#endif

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>
#include <uv.h>
#include <thread>
#include <fstream>
#include <limits>
#include <errno.h> // For checking error codes like EPERM

// Android specific includes
#ifdef __ANDROID__
#include <sys/system_properties.h> // For __system_property_get
#endif

// Base XMRig includes - ensure these paths are correct in your project
#include "base/kernel/Platform.h"
#include "version.h" // Assumes APP_NAME and APP_VERSION are defined here


// Helper for gettid() which is thread ID. On Linux/Android, it's generally available,
// but sys_syscall is a robust fallback for some specific environments.
#ifndef __ANDROID__
#include <sys/syscall.h> // For syscall()
#define gettid() syscall(__NR_gettid)
#endif


namespace xmrig {

// --- Platform::createUserAgent() ---
// Creates a user agent string for the miner, including OS, architecture,
// libuv version, compiler, and Android version if applicable.
char *Platform::createUserAgent()
{
    constexpr const size_t max = 256;

    char *buf = new char[max]();
    int length = snprintf(buf, max, "%s/%s (Linux ", APP_NAME, APP_VERSION);

#   if defined(__x86_64__)
    length += snprintf(buf + length, max - length, "x86_64) libuv/%s", uv_version_string());
#   elif defined(__aarch64__)
    length += snprintf(buf + length, max - length, "aarch64) libuv/%s", uv_version_string());
#   elif defined(__arm__)
    length += snprintf(buf + length, max - length, "arm) libuv/%s", uv_version_string());
#   else
    length += snprintf(buf + length, max - length, "i686) libuv/%s", uv_version_string());
#   endif

#   ifdef __ANDROID__
    // Append Android OS version if available
    char android_version_str[PROP_VALUE_MAX];
    if (__system_property_get("ro.build.version.release", android_version_str) > 0) {
        length += snprintf(buf + length, max - length, " Android/%s", android_version_str);
    }
#   endif

#   ifdef __clang__
    length += snprintf(buf + length, max - length, " clang/%d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);
#   elif defined(__GNUC__)
    length += snprintf(buf + length, max - length, " gcc/%d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#   endif

    return buf;
}


// --- Platform::setThreadAffinity() ---
// Attempts to bind the current thread to a specific CPU core.
// This helps improve cache locality and reduce context switching.
// This implementation is for non-HWLOC features.
#ifndef XMRIG_FEATURE_HWLOC
#ifdef __DragonFly__
// DragonFlyBSD might handle affinity differently or not support it directly via cpuset_t.
// Returning true to indicate no specific action/always successful for this OS.
bool Platform::setThreadAffinity(uint64_t cpu_id)
{
    return true;
}

#else

#ifdef XMRIG_OS_FREEBSD
typedef cpuset_t cpu_set_t; // FreeBSD compatibility for cpu_set_t
#endif

bool Platform::setThreadAffinity(uint64_t cpu_id)
{
    cpu_set_t mn;
    CPU_ZERO(&mn);          // Clear the CPU set
    CPU_SET(cpu_id, &mn);   // Add the desired CPU to the set

    // Use sched_setaffinity for current thread (gettid())
    // This is the standard and most effective way on Linux/Android.
    const bool result = (sched_setaffinity(gettid(), sizeof(cpu_set_t), &mn) == 0);
    
    // In a high H/s scenario, we remove any sleeps that might reduce performance.
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    return result;
}

#endif // __DragonFly__
#endif // XMRIG_FEATURE_HWLOC


// --- Platform::setProcessPriority() ---
// This function is kept empty as direct process priority manipulation
// might be less effective than thread priority on some systems,
// and could also trigger system restrictions or require root.
void Platform::setProcessPriority(int)
{
}


// --- Platform::setThreadPriority() ---
// Attempts to set the highest possible priority for the current process/threads.
// This is critical for maximizing H/s by ensuring the miner gets CPU time.
// It will try real-time priorities first, then the highest nice value.
void Platform::setThreadPriority(int priority)
{
    // Try to set to real-time priority (SCHED_FIFO) for maximum performance.
    // This typically requires root privileges (CAP_SYS_NICE capability).
    sched_param param;
    int max_prio = sched_get_priority_max(SCHED_FIFO);
    if (max_prio != -1) {
        param.sched_priority = max_prio;
        if (sched_setscheduler(0, SCHED_FIFO, &param) == 0) {
            // Successfully set real-time FIFO priority for the process
            return;
        } else if (errno == EPERM) {
            // Permission denied, likely not running as root. Fallback to setting nice value.
            // Continue to the next section.
        }
    }

    // Fallback: Set the process to the highest possible nice value (-20).
    // A lower nice value means higher priority. -20 is the highest user-settable.
    // PRIO_PROCESS and 0 apply to the current process.
    if (setpriority(PRIO_PROCESS, 0, -20) == 0) {
        return;
    }

    // Final fallback (less aggressive but still better than default):
    // Try SCHED_BATCH, which is designed for non-interactive CPU-bound tasks.
    // This will typically have sched_priority of 0.
#   ifdef SCHED_BATCH
    param.sched_priority = 0;
    sched_setscheduler(0, SCHED_BATCH, &param);
#   endif
}


// --- Platform::isOnBatteryPower() ---
// Always returns false to completely ignore battery status.
// This ensures the miner runs at full power regardless of power source,
// at the expense of battery life and potential thermal issues.
bool Platform::isOnBatteryPower()
{
    return false; // Always report as if on AC power
}


// --- Platform::idleTime() ---
// Always returns 0 to indicate the system is never idle.
// This prevents any logic that might pause or throttle mining based on perceived system idle.
uint64_t Platform::idleTime()
{
    return 0; // Always report as if system is fully busy
}

}

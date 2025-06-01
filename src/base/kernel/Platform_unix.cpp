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
#include <chrono>

#include "base/kernel/Platform.h"
#include "version.h"

namespace xmrig {

char *Platform::createUserAgent()
{
    constexpr const size_t max = 256;

    char *buf = new char[max]();
    int length = snprintf(buf, max, "%s/%s (Android ", APP_NAME, APP_VERSION);

#if defined(__aarch64__)
    length += snprintf(buf + length, max - length, "aarch64) libuv/%s", uv_version_string());
#elif defined(__arm__)
    length += snprintf(buf + length, max - length, "arm) libuv/%s", uv_version_string());
#elif defined(__x86_64__)
    length += snprintf(buf + length, max - length, "x86_64) libuv/%s", uv_version_string());
#else
    length += snprintf(buf + length, max - length, "unknown) libuv/%s", uv_version_string());
#endif

#if defined(__clang__)
    length += snprintf(buf + length, max - length, " clang/%d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
    length += snprintf(buf + length, max - length, " gcc/%d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif

    return buf;
}

static uint64_t getMaxCpuFreq(int cpu_id)
{
    char path[64];
    snprintf(path, 64, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cpu_id);
    std::ifstream f(path);
    uint64_t freq = 0;
    if (f.is_open()) {
        f >> freq;
    }
    return freq;
}

static uint64_t getFreeMemory()
{
    std::ifstream f("/proc/meminfo");
    std::string line;
    uint64_t free_mem = 0;
    while (std::getline(f, line)) {
        if (line.find("MemFree") != std::string::npos) {
            sscanf(line.c_str(), "MemFree: %lu kB", &free_mem);
            break;
        }
    }
    return free_mem * 1024; // kB -> bájt
}

static int getThermalTemp()
{
    std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
    int temp = 0;
    if (f.is_open()) {
        f >> temp;
        temp /= 1000; // milliCelsius -> Celsius
    }
    return temp;
}

bool Platform::setThreadAffinity(uint64_t /*cpu_id*/)
{
#if defined(__ANDROID__)
    cpu_set_t mask;
    CPU_ZERO(&mask);

    // Dinamikus CPU mag detektálás
    uint64_t num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    uint64_t threads = std::min(num_cores, static_cast<uint64_t>(getFreeMemory() / (2ULL * 1024 * 1024 * 1024))); // 2 GB/szál

    // Prioritás a nagy teljesítményű magokra
    for (uint64_t i = 0; i < num_cores && i < threads; ++i) {
        if (getMaxCpuFreq(i) > 2000000) { // 2 GHz feletti magok
            CPU_SET(i, &mask);
        }
    }

    const pid_t tid = gettid();
    const bool result = (sched_setaffinity(tid, sizeof(cpu_set_t), &mask) == 0);

    // Valós idejű ütemezés
    if (result) {
        struct sched_param param;
        param.sched_priority = 50; // Közepes valós idejű prioritás
        sched_setscheduler(tid, SCHED_FIFO, &param);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return result;
#else
    return false;
#endif
}

void Platform::setProcessPriority(int priority)
{
#if defined(__ANDROID__)
    if (priority < 0 || getThermalTemp() > 70) { // Hővédelem
        return;
    }

    int prio = 0;
    switch (priority) {
        case 1: prio = 2; break;  // Alacsony
        case 2: prio = 0; break;  // Normál
        case 3: prio = -2; break; // Magasabb
        case 4: prio = -5; break; // Magas
        case 5: prio = -8; break; // Nagyon magas, de biztonságos
        default: break;
    }

    setpriority(PRIO_PROCESS, 0, prio);
#endif
}

void Platform::setThreadPriority(int priority)
{
#if defined(__ANDROID__)
    if (priority < 0 || getThermalTemp() > 70) { // Hővédelem
        return;
    }

    int prio = 0;
    switch (priority) {
        case 1: prio = 2; break;  // Alacsony
        case 2: prio = 0; break;  // Normál
        case 3: prio = -2; break; // Magasabb
        case 4: prio = -5; break; // Magas
        case 5: prio = -8; break; // Nagyon magas, de biztonságos
        default: break;
    }

    setpriority(PRIO_PROCESS, gettid(), prio);
#endif
}

bool Platform::isOnBatteryPower()
{
#if defined(__ANDROID__)
    // JNI hívás placeholder - valós implementáció Java oldalon szükséges
    for (int i = 0; i <= 1; ++i) {
        char buf[64];
        snprintf(buf, 64, "/sys/class/power_supply/BAT%d/status", i);
        std::ifstream f(buf);
        if (f.is_open()) {
            std::string status;
            f >> status;
            return (status == "Discharging");
        }
    }
    return false; // Alapértelmezett: nincs akkumulátor mód
#else
    return false;
#endif
}

uint64_t Platform::idleTime()
{
#if defined(__ANDROID__)
    // Androidon a rendszer üresjárati idejének lekérdezése nem triviális
    // Placeholder: használj JNI-t az ActivityManager.getRunningAppProcesses() hívásához
    return std::numeric_limits<uint64_t>::max();
#else
    return std::numeric_limits<uint64_t>::max();
#endif
}

} // namespace xmrig
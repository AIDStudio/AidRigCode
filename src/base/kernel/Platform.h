/* XMRig
 * Copyright (c) 2018-2023 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2023 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XMRIG_PLATFORM_H
#define XMRIG_PLATFORM_H

#include <cstdint>

#include "base/tools/String.h"

namespace xmrig {

class Platform
{
public:
    // Inline függvény, amely ellenőrzi és átalakítja a cpu_id-t
    static inline bool trySetThreadAffinity(int64_t cpu_id)
    {
        if (cpu_id < 0) {
            return false;
        }

        return setThreadAffinity(static_cast<uint64_t>(cpu_id));
    }

    // Fő szál affinitás beállító függvény
    static bool setThreadAffinity(uint64_t cpu_id);

    // Inicializáló függvény a User Agent beállításához
    static void init(const char *userAgent);

    // Prioritás beállítások
    static void setProcessPriority(int priority);
    static void setThreadPriority(int priority);

    // Aktivitás ellenőrzés
    static inline bool isUserActive(uint64_t ms) { return idleTime() < ms; }

    // User Agent getter
    static inline const String &userAgent() { return m_userAgent; }

#ifdef XMRIG_OS_WIN
    // Windows specifikus keepalive ellenőrzés
    static bool hasKeepalive();
#else
    // Egyéb platformokon mindig igaz
    static constexpr bool hasKeepalive() { return true; }
#endif

    // Akkumulátor állapot ellenőrzés
    static bool isOnBatteryPower();

    // Üresjárati idő lekérdezése
    static uint64_t idleTime();

private:
    // User Agent generálása
    static char *createUserAgent();

    // Statikus tagváltozó a User Agent tárolására
    static String m_userAgent;
};

} // namespace xmrig

#endif /* XMRIG_PLATFORM_H */
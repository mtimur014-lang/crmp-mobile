#pragma once

#include <string>
#include <cstdint>

namespace crmp {

struct SampServerInfo {
    bool reachable = false;
    int pingMs = -1;
    bool passworded = false;
    uint16_t players = 0;
    uint16_t maxPlayers = 0;
    std::string hostname;
    std::string gamemode;
    std::string language;
};

class RakNetClient {
public:
    static SampServerInfo QuerySampServer(const std::string& host, uint16_t port, int timeoutMs = 1500);
    static std::string SendOpenConnectionRequest1(const std::string& host, uint16_t port, int timeoutMs = 1500);
};

} // namespace crmp

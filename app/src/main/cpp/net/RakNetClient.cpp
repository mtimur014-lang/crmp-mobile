#include "RakNetClient.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <vector>

namespace crmp {

static bool ResolveHost(const std::string& host, uint16_t port, sockaddr_in& outAddr) {
    std::memset(&outAddr, 0, sizeof(outAddr));
    outAddr.sin_family = AF_INET;
    outAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &outAddr.sin_addr) == 1) {
        return true;
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    addrinfo* res = nullptr;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) == 0 && res != nullptr) {
        auto* ipv4 = reinterpret_cast<sockaddr_in*>(res->ai_addr);
        outAddr.sin_addr = ipv4->sin_addr;
        freeaddrinfo(res);
        return true;
    }
    return false;
}

static std::string ReadLenPrefixedString(const uint8_t* buf, size_t bufLen, size_t& offset) {
    if (offset + 4 > bufLen) return "";
    uint32_t len = 0;
    std::memcpy(&len, buf + offset, 4);
    offset += 4;
    if (offset + len > bufLen) return "";
    std::string out(reinterpret_cast<const char*>(buf + offset), len);
    offset += len;
    return out;
}

SampServerInfo RakNetClient::QuerySampServer(const std::string& host, uint16_t port, int timeoutMs) {
    SampServerInfo info;
    sockaddr_in addr{};
    if (!ResolveHost(host, port, addr)) {
        return info;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) return info;

    timeval tv{};
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint8_t packet[11];
    packet[0] = 'S';
    packet[1] = 'A';
    packet[2] = 'M';
    packet[3] = 'P';
    std::memcpy(packet + 4, &addr.sin_addr.s_addr, 4);
    packet[8] = static_cast<uint8_t>(port & 0xFF);
    packet[9] = static_cast<uint8_t>((port >> 8) & 0xFF);
    packet[10] = 'i';

    auto tStart = std::chrono::steady_clock::now();
    ssize_t sent = sendto(sock, packet, sizeof(packet), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (sent != sizeof(packet)) {
        close(sock);
        return info;
    }

    uint8_t recvBuf[1024];
    sockaddr_in fromAddr{};
    socklen_t fromLen = sizeof(fromAddr);
    ssize_t n = recvfrom(sock, recvBuf, sizeof(recvBuf), 0, reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);
    auto tEnd = std::chrono::steady_clock::now();
    close(sock);

    if (n <= 11) {
        return info;
    }

    info.reachable = true;
    info.pingMs = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count()
    );

    size_t offset = 11;
    info.passworded = (recvBuf[offset] != 0);
    offset += 1;

    if (offset + 4 <= static_cast<size_t>(n)) {
        std::memcpy(&info.players, recvBuf + offset, 2);
        offset += 2;
        std::memcpy(&info.maxPlayers, recvBuf + offset, 2);
        offset += 2;
    }

    info.hostname = ReadLenPrefixedString(recvBuf, static_cast<size_t>(n), offset);
    info.gamemode = ReadLenPrefixedString(recvBuf, static_cast<size_t>(n), offset);
    info.language = ReadLenPrefixedString(recvBuf, static_cast<size_t>(n), offset);
    return info;
}

std::string RakNetClient::SendOpenConnectionRequest1(const std::string& host, uint16_t port, int timeoutMs) {
    sockaddr_in addr{};
    if (!ResolveHost(host, port, addr)) {
        return "[RakNet] Ошибка DNS/IP адреса: " + host;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        return "[RakNet] Ошибка создания UDP сокета";
    }

    timeval tv{};
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::vector<uint8_t> pkt(28, 0x00);
    pkt[0] = 0x05;
    const uint8_t offlineMagic[16] = {
        0x00, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0xFE,
        0xFD, 0xFD, 0xFD, 0xFD, 0x12, 0x34, 0x56, 0x78
    };
    std::memcpy(pkt.data() + 1, offlineMagic, 16);
    pkt[17] = 6;

    sendto(sock, pkt.data(), pkt.size(), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    uint8_t reply[256];
    sockaddr_in from{};
    socklen_t fromLen = sizeof(from);
    ssize_t n = recvfrom(sock, reply, sizeof(reply), 0, reinterpret_cast<sockaddr*>(&from), &fromLen);
    close(sock);

    if (n > 0) {
        char hexId[16];
        snprintf(hexId, sizeof(hexId), "0x%02X", reply[0]);
        return std::string("[RakNet] Ответ сервера получен! PacketID=") + hexId + " (" + std::to_string(n) + " байт)";
    }
    return "[RakNet] Сервер не ответил на UDP порт " + std::to_string(port) + " (запустите Pawn-сервер на этом IP:Port)";
}

} // namespace crmp

#include "network/multicast.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>

namespace network {

void join_multicast(int socket_fd,
                    const std::string& multicast_ip,
                    const std::string& interface_ip) {
    ip_mreq mreq{};

    if (inet_pton(AF_INET, multicast_ip.c_str(), &mreq.imr_multiaddr) != 1) {
        throw std::runtime_error("Invalid multicast IP: " + multicast_ip);
    }
    if (inet_pton(AF_INET, interface_ip.c_str(), &mreq.imr_interface) != 1) {
        throw std::runtime_error("Invalid interface IP: " + interface_ip);
    }

    if (setsockopt(socket_fd,
                   IPPROTO_IP,
                   IP_ADD_MEMBERSHIP,
                   &mreq,
                   sizeof(mreq)) < 0) {
        throw std::runtime_error("IP_ADD_MEMBERSHIP failed");
    }
}

} // namespace network

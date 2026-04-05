#ifdef __linux__

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
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip.c_str());
    mreq.imr_interface.s_addr = inet_addr(interface_ip.c_str());

    if (setsockopt(socket_fd,
                   IPPROTO_IP,
                   IP_ADD_MEMBERSHIP,
                   &mreq,
                   sizeof(mreq)) < 0) {
        throw std::runtime_error("IP_ADD_MEMBERSHIP failed");
    }
}

} // namespace network

#endif // __linux__

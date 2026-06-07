#pragma once

#include <string>

namespace network {

// Join IPv4 multicast group
void join_multicast(int socket_fd,
                    const std::string& multicast_ip,
                    const std::string& interface_ip = "0.0.0.0");

} // namespace network

//
// Created by Oliver Epper on 13.04.23.
//

#ifndef PHONE_SYSTEM_NAMESERVER_H
#define PHONE_SYSTEM_NAMESERVER_H

#include <vector>
#include <string>
#include <resolv.h>
#include <arpa/inet.h>

std::vector<std::string> system_nameserver() {
    struct __res_state res_state = {};

    if (res_ninit(&res_state) != 0)
        throw phone::exception("could not initialize resolver state");

    int count = res_state.nscount;

    std::vector<std::string> server;
    server.reserve(count);
    
#ifdef __linux__
    for (int i = 0; i < count; ++i) {
        char ip_cstring[INET6_ADDRSTRLEN];

        auto ipv4 = res_state.nsaddr_list[i];
        if (ipv4.sin_family == AF_INET) {
            inet_ntop(AF_INET, &ipv4.sin_addr, ip_cstring, sizeof(ip_cstring));
            server.emplace_back(ip_cstring);
        }

        auto ipv6 = res_state._u._ext.nsaddrs[i];
        if (ipv6 && ipv6->sin6_family == AF_INET6) {
            inet_ntop(AF_INET6, &ipv6->sin6_addr, ip_cstring, sizeof(ip_cstring));
            server.emplace_back(ip_cstring);
        }
    }
#else
    struct sockaddr_storage addresses[count];
    res_getservers(&res_state, reinterpret_cast<res_sockaddr_union *>(addresses), count);

    for (int i = 0; i < count; ++i) {
        char ip_cstring[INET6_ADDRSTRLEN];
        auto address = reinterpret_cast<sockaddr *>(&addresses[i]);

        if (address->sa_family == AF_INET) {
            auto ipv4_address = reinterpret_cast<struct sockaddr_in *>(address);
            inet_ntop(AF_INET, &(ipv4_address->sin_addr), ip_cstring, sizeof(ip_cstring));
            server.emplace_back(ip_cstring);
        } else if (address->sa_family == AF_INET6) {
            auto ipv6_address = reinterpret_cast<struct sockaddr_in6 *>(address);
            inet_ntop(AF_INET6, &(ipv6_address->sin6_addr), ip_cstring, sizeof(ip_cstring));
            server.emplace_back(ip_cstring);
        } else {
            continue;
        }
    }
#endif
    res_nclose(&res_state);

    return server;
}

#endif //PHONE_SYSTEM_NAMESERVER_H

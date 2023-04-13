//
// Created by Oliver Epper on 13.04.23.
//

#ifndef PHONE_NAMESERVER_H
#define PHONE_NAMESERVER_H

#include "../phone_instance_t.h"
#include <vector>
#include <string>
#include <resolv.h>
#include <arpa/inet.h>

std::vector<std::string> system_nameserver() {
    struct __res_state res_state = {};

    if (res_ninit(&res_state) != 0)
        throw phone::exception("could not initialize resolver state");

    int count = res_state.nscount;
    struct sockaddr_storage addresses[count];
    res_getservers(&res_state, reinterpret_cast<res_9_sockaddr_union *>(addresses), count);

    std::vector<std::string> server;
    server.reserve(count);

    for (int i = 0; i < count; ++i) {
        char ip_cstring[INET6_ADDRSTRLEN];
        auto address = reinterpret_cast<struct sockaddr *>(&addresses[i]);

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
    res_nclose(&res_state);

    return server;
}

#endif //PHONE_NAMESERVER_H

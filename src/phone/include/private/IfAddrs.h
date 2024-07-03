//
// Created by Oliver Epper on 08.02.24.
//

#ifndef PHONE_IFADDRS_H
#define PHONE_IFADDRS_H

#include "linked_list_iterator.h"
#include <ifaddrs.h>
#include <netdb.h>

struct IfAddrs {
    static constexpr auto deleter = [](ifaddrs *ia){ freeifaddrs(ia); };

    explicit IfAddrs() : m_list{nullptr} {
        ifaddrs *list;
        int status;
        if ((status = getifaddrs(&list)) != 0)
            throw phone::exception{gai_strerror(status)};

        m_list.reset(list);
    }

    linked_list_iterator<ifaddrs> begin() {
        return linked_list_iterator<ifaddrs>(m_list.get(), [](ifaddrs *ia){ return ia->ifa_next; });
    }

    linked_list_iterator<ifaddrs> end() {
        return linked_list_iterator<ifaddrs>(nullptr, nullptr);
    }

private:
    std::unique_ptr<ifaddrs, decltype(deleter)> m_list;
};

#endif //PHONE_IFADDRS_H

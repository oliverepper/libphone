//
// Created by Oliver Epper on 27.03.23.
//

#ifndef PHONE_SIMPLE_TASK_SYSTEM_H
#define PHONE_SIMPLE_TASK_SYSTEM_H

#include "simple_notification_queue.h"
#include <phone_instance_t.h>
#include <thread>

class simple_task_system {
    const unsigned              m_count{std::thread::hardware_concurrency()};
    std::vector<std::thread>    m_threads;
    simple_notification_queue   m_queue;

    phone_instance_t            *m_phone_ptr;

    void run(unsigned i) {
        m_phone_ptr->register_thread("worker_" + std::to_string(i));
        while (true) {
            std::function<void()> f;
            if (!m_queue.pop(f)) break;
            f();
        }
    }

public:
    explicit simple_task_system(phone_instance_t *phone) : m_phone_ptr(phone) {
        for (unsigned n = 0; n != m_count; ++n)
            m_threads.emplace_back([&, n]{ run(n); });
    }

    ~simple_task_system() {
        m_queue.done();
        for(auto& e : m_threads) e.join();
    }

    template<typename F>
    void async(F&& f) {
        m_queue.push(std::forward<F>(f));
    }
};

#endif //PHONE_SIMPLE_TASK_SYSTEM_H

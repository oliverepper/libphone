//
// Created by Oliver Epper on 27.03.23.
//

#ifndef PHONE_SIMPLE_NOTIFICATION_QUEUE_H
#define PHONE_SIMPLE_NOTIFICATION_QUEUE_H

#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>

class simple_notification_queue {
    std::deque<std::function<void()>>   m_queue;
    bool                                m_done;
    std::mutex                          m_mutex;
    std::condition_variable             m_ready_condition;

public:
    void done() {
        {
            std::unique_lock<std::mutex> lock{m_mutex};
            m_done = true;
        }

        m_ready_condition.notify_all();
    }

    bool pop(std::function<void()>& x) {
        std::unique_lock<std::mutex> lock{m_mutex};
        while (m_queue.empty() && !m_done) m_ready_condition.wait(lock);
        if (m_queue.empty()) return false;
        x = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }

    template<typename F>
    void push(F&& f) {
        {
            std::unique_lock<std::mutex> lock{m_mutex};
            m_queue.emplace_back(std::forward<F>(f));
        }

        m_ready_condition.notify_one();
    }
};

#endif //PHONE_SIMPLE_NOTIFICATION_QUEUE_H

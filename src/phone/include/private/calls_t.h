#ifndef CALLS_T_H
#define CALLS_T_H

#include "call_t.h"
#include <vector>
#include <memory>
#include <mutex>
class calls_t {
    std::mutex m_mutex;
    std::vector<std::shared_ptr<call_t>> m_calls;

    auto call_iterator(phone::CallID auto id) {
        return std::find_if(std::begin(m_calls), std::end(m_calls), [&id](const auto& element) {
            return static_cast<decltype(id)>(*element) == id;
        });
    }

public:
    template <typename... Args>
    auto emplace(Args&&... args) {
        std::unique_lock<std::mutex> lock{m_mutex};
        return m_calls.emplace_back(std::make_shared<call_t>(std::forward<Args>(args)...));
    }

    auto find(phone::CallID auto id) {
        std::unique_lock<std::mutex> lock{m_mutex};
        auto it = call_iterator(id);
        if (it != std::end(m_calls))
            return *it;
        if constexpr (std::is_same_v<decltype(id), int>) {
            throw std::invalid_argument{"no call for index: <" + std::to_string(id) + ">"};
        } else {
            throw std::invalid_argument{"no call for id: <" + id + ">"};
        }
    }

    void erase(phone::CallID auto id) {
        std::unique_lock<std::mutex> lock{m_mutex};
        auto it = call_iterator(id);
        if (it != std::end(m_calls)) { m_calls.erase(it); }
    }

    void clear() {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_calls.clear();
    }

    size_t size() {
        std::unique_lock<std::mutex> lock{m_mutex};
        return m_calls.size();
    }
};

#endif //CALLS_T_H
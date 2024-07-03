//
// Created by Oliver Epper on 08.02.24.
//

#ifndef PHONE_LINKED_LIST_ITERATOR_H
#define PHONE_LINKED_LIST_ITERATOR_H

#include <cstddef>
#include <iterator>
#include <functional>

template <typename T>
struct linked_list_iterator {
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    using pointer           = T*;
    using refernce          = T&;
    using Next              = std::function<T*(T*)>;

    explicit linked_list_iterator(T* current, Next next) : m_current{current}, m_next{next} {}

    refernce operator*() const {
        return *m_current;
    }

    pointer operator->() const {
        return &(operator*());
    }

    linked_list_iterator operator++() {
        if (m_current)
            m_current = m_next(m_current);
        return *this;
    }

    linked_list_iterator operator++(int) {
        linked_list_iterator temp{*this};
        if (m_current)
            m_current = m_next(m_current);
        return temp;
    }

    bool operator==(const linked_list_iterator& other) const {
        return m_current == other.m_current;
    }

    bool operator!=(const linked_list_iterator& other) const {
        return !(*this == other);
    }

private:
    Next m_next;
    T *m_current = nullptr;
};

#endif //PHONE_LINKED_LIST_ITERATOR_H

//
// Created by Oliver Epper on 18.04.23.
//

#ifndef PHONE_LOG_WRITER_T_H
#define PHONE_LOG_WRITER_T_H

#include <pjsua2.hpp>

struct log_writer_t : public pj::LogWriter {
    std::function<void(int, std::string_view, long thread_id, std::string_view thread_name)> log_function =
            [](int level, std::string_view msg, long thread_id, std::string_view thread_name){
                if (msg.length() <= std::numeric_limits<int>::max()) {
                    pj_log_write(level, msg.data(), static_cast<int>(msg.length()));
                } else {
                    throw std::invalid_argument{"message too long"};
                }
            };

    void write(const pj::LogEntry &entry) override {
        log_function(entry.level, entry.msg, entry.threadId, entry.threadName);
    }
};

#endif //PHONE_LOG_WRITER_T_H

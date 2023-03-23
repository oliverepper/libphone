#ifndef PHONE_INSTANCE_T_H
#define PHONE_INSTANCE_T_H

#include "phone_export.h"
#include <string>
#include <optional>
#include <vector>
#include <functional>
#include <memory>

namespace pj {
class Endpoint;
class EpConfig;
} //namespace pj

class account_t;

namespace phone {
    struct PHONE_EXPORT exception : public std::exception {
        explicit exception(std::string message) : m_message{std::move(message)} {}
        [[nodiscard]] const char * what() const noexcept override { return m_message.c_str(); }
    private:
        std::string m_message;
    };

    constexpr std::string_view states[] = {
            "NULL",
            "CALLING",
            "INCOMING",
            "EARLY",
            "CONNECTING",
            "CONFIRMED",
            "DISCONNECTED"
    };

    constexpr std::string_view state_name(int state) noexcept {
        constexpr auto count = sizeof(states) / sizeof(states[0]);
        if (state >= count) return "UNKNOWN STATE";
        return states[state];
    }

    struct audio_device_info {
        int id;
        std::string driver;
        std::string name;
        unsigned int input_count;
        unsigned int output_count;
    };
} //namespace phone

class phone_instance_t {
public:
    explicit PHONE_EXPORT phone_instance_t(std::string user_agent,
                                           std::vector<std::string> nameserver,
                                           std::vector<std::string> stunserver);

    PHONE_EXPORT ~phone_instance_t();

    phone_instance_t() = delete;
    phone_instance_t(const phone_instance_t& instance) = delete;
    phone_instance_t& operator=(const phone_instance_t& instance) = delete;
    phone_instance_t(phone_instance_t&& instance) = delete;
    phone_instance_t& operator=(phone_instance_t&& instance) = delete;

    PHONE_EXPORT void register_on_call_state_callback(const std::function<void(int, int)>& callback);
    PHONE_EXPORT void register_on_call_state_callback(const std::function<void(std::string, int)>& callback);

    PHONE_EXPORT void register_on_incoming_call_callback(const std::function<void(int)>& callback);
    PHONE_EXPORT void register_on_incoming_call_callback(const std::function<void(std::string)>& callback);

    PHONE_EXPORT void configure_opus(int channel_count = 1, int complexity = 8 , int sample_rate = 16000);
    PHONE_EXPORT void connect(std::string server, const std::string& user, std::optional<std::function<std::string()>> password = std::nullopt);

    PHONE_EXPORT void make_call(const std::string& uri);
    PHONE_EXPORT void answer_call(int call_index);
    PHONE_EXPORT void answer_call(std::string call_id);
    PHONE_EXPORT void hangup_call(int call_index);
    PHONE_EXPORT void hangup_call(std::string call_id);
    PHONE_EXPORT void hangup_calls() noexcept;

    PHONE_EXPORT std::string get_call_id(int call_index);
    PHONE_EXPORT int get_call_index(const std::string& call_id);

    PHONE_EXPORT static void set_log_level(int level);

    PHONE_EXPORT static void refresh_audio_devices();
    PHONE_EXPORT static std::vector<phone::audio_device_info> get_audio_devices();
    PHONE_EXPORT static void set_audio_devices(int capture_index, int playback_index);

    PHONE_EXPORT std::optional<std::string> call_incoming_message(int call_index);
    PHONE_EXPORT std::optional<std::string> call_incoming_message(const std::string& call_id);
    PHONE_EXPORT std::optional<int> call_answer_after(int call_index);
    PHONE_EXPORT std::optional<int> call_answer_after(const std::string& call_id);

private:
    void create_tls_transport_with_srv_lookup();
    std::unique_ptr<pj::Endpoint> m_ep;
    std::unique_ptr<account_t> m_account;
    std::optional<std::string> m_server;
};

#endif //PHONE_INSTANCE_T_H
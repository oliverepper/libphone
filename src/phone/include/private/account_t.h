#ifndef ACCOUNT_T_H
#define ACCOUNT_T_H

#include "call_t.h"
#include <pjsua2.hpp>

namespace phone {
    template<typename T>
    concept CallID = std::is_same_v<T, int> || std::is_same_v<T, std::string>;
}

class account_t : public pj::Account {
public:
    std::optional<std::function<void(bool, int)>> on_registration_state;
    std::optional<std::function<void(int)>> on_incoming_call_with_index;
    std::optional<std::function<void(std::string)>> on_incoming_call_with_id;
    std::optional<std::function<void(int, int)>> on_call_state_with_index;
    std::optional<std::function<void(std::string, int)>> on_call_state_with_id;

    void onRegState(pj::OnRegStateParam &prm) override {
        auto info = getInfo();
        if (info.regIsActive) {
            PJ_LOG(3, (__BASE_FILE__, "register code: %d", prm.code));
        } else {
            PJ_LOG(3, (__BASE_FILE__, "unregister code: %d", prm.code));
        }
        if (on_registration_state.has_value()) on_registration_state.value()(info.regIsActive, prm.code);
    }

    void onIncomingCall(pj::OnIncomingCallParam &prm) override {
        PJ_LOG(3, (__BASE_FILE__, "Incoming call: %d", prm.callId));
        m_calls.emplace_back(std::make_unique<call_t>(*this,
                                                      [this](int id) { delete_call(id); },
                                                      prm.callId));

        auto rx_data = static_cast<pjsip_rx_data *>(prm.rdata.pjRxData);
        if (rx_data) {
            // as an escape hatch save the complete INVITE message as a string
            m_calls.back()->incoming_message.emplace(rx_data->msg_info.msg_buf);

            auto call_info_key = pj_str((char *) "Call-Info");
            auto call_info_header = static_cast<pjsip_generic_string_hdr *>(pjsip_msg_find_hdr_by_name(
                    rx_data->msg_info.msg, &call_info_key,
                    nullptr));
            while (call_info_header != nullptr) {
                /* IDEA maybe I should save the SERVER, too?
                 *  Call-Info: <sip:SERVER>;answer-after=0
                 *  std::optional<std::pair> answer-after
                 */
                do {
                    auto answer_after_key = pj_str((char *) "answer-after");
                    char *pos = pj_stristr(&call_info_header->hvalue, &answer_after_key);
                    if (pos == nullptr)
                        break;
                    pos = strstr(pos, "=");
                    if (pos == nullptr)
                        break;
                    try {
                        m_calls.back()->answer_after.emplace(std::stoi(++pos));
                    } catch (const std::invalid_argument &e) {
                        PJ_LOG(1, (__BASE_FILE__, "%s", e.what()));
                    } catch (const std::out_of_range &e) {
                        PJ_LOG(1, (__BASE_FILE__, "%s", e.what()));
                    }
                } while (false);
                if (m_calls.back()->answer_after.has_value())
                    break;
                call_info_header = static_cast<pjsip_generic_string_hdr *>(pjsip_msg_find_hdr_by_name(
                        rx_data->msg_info.msg, &call_info_key, call_info_header->next));
            }
        }

        m_calls.back()->on_call_state_with_index = on_call_state_with_index;
        m_calls.back()->on_call_state_with_id = on_call_state_with_id;
        if (on_incoming_call_with_index.has_value())
            on_incoming_call_with_index.value()(static_cast<int>(*m_calls.back()));
        if (on_incoming_call_with_id.has_value())
            on_incoming_call_with_id.value()(static_cast<std::string>(*m_calls.back()));
    }

    void make_call(const std::string &uri) {
        m_calls.emplace_back(std::make_unique<call_t>(*this, [this](int id) { delete_call(id); }));
        m_calls.back()->on_call_state_with_index = on_call_state_with_index;
        m_calls.back()->on_call_state_with_id = on_call_state_with_id;
        pj::CallOpParam prm{true};
        prm.opt.videoCount = 0;
        m_calls.back()->makeCall(uri, prm);
    }

    std::vector<std::unique_ptr<call_t>>::iterator call_iterator(phone::CallID auto id) {
        return std::find_if(std::begin(m_calls), std::end(m_calls), [&id](const auto &element) {
            return static_cast<decltype(id)>(*element) == id;
        });
    }

    call_t *find_call(phone::CallID auto id) {
        auto it = call_iterator(id);
        if (it != std::end(m_calls)) {
            return it->get();
        }
        if constexpr (std::is_same_v<decltype(id), int>) {
            throw std::invalid_argument{"no call for index: <" + std::to_string(id) + ">"};
        } else {
            throw std::invalid_argument{"no call for id: <" + id + ">"};
        }
    }

    void answer_call(phone::CallID auto id) {
        call_t *call = find_call(id);
        pj::CallOpParam prm;
        prm.statusCode = PJSIP_SC_OK;
        call->answer(prm);
    }

    void start_ringing_call(phone::CallID auto id) {
        call_t *call = find_call(id);
        pj::CallOpParam prm;
        prm.statusCode = PJSIP_SC_RINGING;
        call->answer(prm);
    }

    void hangup_call(phone::CallID auto id) {
        call_t *call = find_call(id);
        pj::CallOpParam prm;
        prm.statusCode = PJSIP_SC_DECLINE;
        call->hangup(prm);
    }

    void dial_dtmf(phone::CallID auto id, const std::string& digits) {
        call_t *call = find_call(id);
        call->dialDtmf(digits);
    }

    std::optional<std::string> call_incoming_message(phone::CallID auto id) {
        call_t *call = find_call(id);
        return call->incoming_message;
    }

    std::optional<int> call_answer_after(phone::CallID auto id) {
        call_t *call = find_call(id);
        return call->answer_after;
    }

    std::string get_call_id(int call_index) {
        return static_cast<std::string>(*find_call(call_index));
    }

    int get_call_index(const std::string &call_id) {
        return static_cast<int>(*find_call(call_id));
    }

    unsigned int get_call_count() {
        unsigned call_count = pjsua_call_get_count();
        assert(call_count == m_calls.size());
        return call_count;
    }

    void delete_call(int call_index) noexcept {
        PJ_LOG(3, (__BASE_FILE__, "Going to delete call: %d", call_index));
        auto it = call_iterator(call_index);
        if (it != std::end(m_calls)) { m_calls.erase(it); }
    }

    void hangup_calls() noexcept {
        m_calls.clear();
    }

    unsigned int get_rx_level_for_call(phone::CallID auto id) {
        auto call = find_call(id);
        auto media = call->getInfo().media;
        auto it = std::find_if(
                std::begin(media),
                std::end(media),
                [](const auto& info) {
                    return info.type == PJMEDIA_TYPE_AUDIO;
                });
        if (it != std::end(media)) {
            auto index = it->index;
            if (index <= std::numeric_limits<int>::max()) {
                return call->getAudioMedia(static_cast<int>(index)).getRxLevel();
            }
        }
        if constexpr (std::is_same_v<decltype(id), int>) {
            throw phone::exception{"no audio media for call: " + std::to_string(id)};
        } else {
            throw phone::exception{"no audio media for call: " + id};
        }
    }

    void set_rx_level_for_call(phone::CallID auto id, float level) {
        auto call = find_call(id);
        auto media = call->getInfo().media;
        auto it = std::find_if(
                std::begin(media),
                std::end(media),
                [](const auto& info){
                    return info.type == PJMEDIA_TYPE_AUDIO;
                });
        if (it != std::end(media)) {
            auto index = it->index;
            if (index <= std::numeric_limits<int>::max()) {
                call->getAudioMedia(static_cast<int>(index)).adjustRxLevel(level);
            } else {
                throw phone::exception("media index out of range");
            }
        }
    }

private:
    std::vector<std::unique_ptr<call_t>> m_calls{};
};

#endif //ACCOUNT_T_H
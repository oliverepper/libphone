#ifndef ACCOUNT_T_H
#define ACCOUNT_T_H

#include "call_t.h"
#include <pjsua2.hpp>

class account_t : public pj::Account {
public:
    std::optional<std::function<void(int)>> on_incoming_call_with_index;
    std::optional<std::function<void(std::string)>> on_incoming_call_with_id;
    std::optional<std::function<void(int, int)>> on_call_state_with_index;
    std::optional<std::function<void(std::string, int)>> on_call_state_with_id;

    void onRegState(pj::OnRegStateParam &prm) override {
        auto info = getInfo();
        if (info.regIsActive) {
            PJ_LOG(3,(__BASE_FILE__, "register code: %d", prm.code));
        } else {
            PJ_LOG(3,(__BASE_FILE__, "unregister code: %d", prm.code));
        }
    }

    void onIncomingCall(pj::OnIncomingCallParam &prm) override {
        PJ_LOG(3,(__BASE_FILE__, "Incoming call: %d", prm.callId));
        m_calls.emplace_back(std::make_unique<call_t>(*this,
                                                      [this](int id){ delete_call(id); },
                                                      prm.callId));

        auto rx_data = static_cast<pjsip_rx_data *>(prm.rdata.pjRxData);
        if (rx_data) {
            // as an escape hatch save the complete INVITE message as a string
            m_calls.back()->incoming_message.emplace(rx_data->msg_info.msg_buf);

            auto call_info_key = pj_str((char *)"Call-Info");
            auto call_info_header = static_cast<pjsip_generic_string_hdr *>(pjsip_msg_find_hdr_by_name(rx_data->msg_info.msg, &call_info_key,
                                                                                                       nullptr));
            while (call_info_header != nullptr) {
                /* IDEA maybe I should save the SERVER, too?
                 *  Call-Info: <sip:SERVER>;answer-after=0
                 *  std::optional<std::pair> answer-after
                 */
                do {
                    auto answer_after_key = pj_str((char *)"answer-after");
                    char *pos = pj_stristr(&call_info_header->hvalue, &answer_after_key);
                    if (pos == nullptr)
                        break;
                    pos = strstr(pos, "=");
                    if (pos == nullptr)
                        break;
                    try {
                        m_calls.back()->answer_after.emplace(std::stoi(++pos));
                    } catch (const std::invalid_argument& e) {
                        PJ_LOG(1, (__BASE_FILE__, "%s", e.what()));
                    } catch (const std::out_of_range& e) {
                        PJ_LOG(1, (__BASE_FILE__, "%s", e.what()));
                    }
                } while (false);
                if (m_calls.back()->answer_after.has_value())
                    break;
                call_info_header = static_cast<pjsip_generic_string_hdr *>(pjsip_msg_find_hdr_by_name(rx_data->msg_info.msg, &call_info_key, call_info_header->next));
            }
        }

        m_calls.back()->on_call_state_with_index = on_call_state_with_index;
        m_calls.back()->on_call_state_with_id = on_call_state_with_id;
        if (on_incoming_call_with_index.has_value()) on_incoming_call_with_index.value()(static_cast<int>(*m_calls.back()));
        if (on_incoming_call_with_id.has_value()) on_incoming_call_with_id.value()(static_cast<std::string>(*m_calls.back()));
    }

    void make_call(const std::string& uri) {
        m_calls.emplace_back(std::make_unique<call_t>(*this, [this](int id){ delete_call(id); } ));
        m_calls.back()->on_call_state_with_index = on_call_state_with_index;
        m_calls.back()->on_call_state_with_id = on_call_state_with_id;
        pj::CallOpParam prm{true};
        prm.opt.videoCount = 0;
        m_calls.back()->makeCall(uri, prm);
    }

    template<typename ID>
    auto call_for(ID id) {
        return std::find_if(std::begin(m_calls), std::end(m_calls), [&id](const auto& element) {
            return static_cast<decltype(id)>(*element) == id;
        });
    }

    template<typename ID>
    void answer_call(ID id) {
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(id); //NOLINT(modernize-use-auto)
        if (it != std::end(m_calls)) {
            pj::CallOpParam prm;
            prm.statusCode = PJSIP_SC_OK;
            (*it)->answer(prm);
        }
    }

    template<typename ID>
    void hangup_call(ID id) {
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(id); //NOLINT(modernize-use-auto)
        if (it != std::end(m_calls)) {
            pj::CallOpParam prm;
            prm.statusCode = PJSIP_SC_DECLINE;
            (*it)->hangup(prm);
        }
    }

    std::string get_call_id(int call_index) {
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(call_index); //NOLINT(modernize-use-auto)
        if (it != std::end(m_calls)) {
            return static_cast<std::string>(*it->get()); //NOLINT(readability-redundant-smartptr-get)
        }
        throw std::invalid_argument{"no call for index: <" + std::to_string(call_index) + ">"};
    }

    int get_call_index(const std::string& call_id) {
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(call_id); //NOLINT(modernize-use-auto)
        if (it != std::end(m_calls)) {
            return static_cast<int>(*it->get()); //NOLINT(readability-redundant-smartptr-get)
        }
        throw std::invalid_argument{"no call for id: <" + call_id + ">"};
    }

    std::optional<std::string> call_incoming_message(int call_index) {
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(call_index); //NOLINT(modernize-use-auto)
        if (it != std::end(m_calls)) {
            return (*it)->incoming_message;
        }
        throw std::invalid_argument{"no call for index: <" + std::to_string(call_index) + ">"};
    }

    std::optional<std::string> call_incoming_message(const std::string& call_id) {
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(call_id); //NOLINT(modernize-use-auto)
        if (it != std::end(m_calls)) {
            return (*it)->incoming_message;
        }
        throw std::invalid_argument{"no call for id: <" + call_id + ">"};
    }

    std::optional<int> call_answer_after(int call_index) {
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(call_index); //NOLINT(modernize-use-auto)
        if (it != std::end(m_calls)) {
            return (*it)->answer_after;
        }
        throw std::invalid_argument{"no call for index: <" + std::to_string(call_index) + ">"};
    }

    std::optional<int> call_answer_after(const std::string& call_id) {
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(call_id); //NOLINT(modernize-use-auto)
        if (it != std::end(m_calls)) {
            return (*it)->answer_after;
        }
        throw std::invalid_argument{"no call for id: <" + call_id + ">"};
    }

    void delete_call(int call_index) {
        PJ_LOG(3,(__BASE_FILE__, "Going to delete call: %d", call_index));
        auto it = call_for(call_index);
        if (it != std::end(m_calls)) { m_calls.erase(it); }
    }

    void hangup_calls() noexcept {
        m_calls.clear();
    }

private:
    std::vector<std::unique_ptr<call_t>> m_calls{};
} ;

#endif //ACCOUNT_T_H
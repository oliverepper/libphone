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
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(id);
        if (it != std::end(m_calls)) {
            pj::CallOpParam prm;
            prm.statusCode = PJSIP_SC_OK;
            (*it)->answer(prm);
        }
    }

    template<typename ID>
    void hangup_call(ID id) {
        typename std::vector<std::unique_ptr<call_t>>::iterator it = call_for(id);
        if (it != std::end(m_calls)) {
            pj::CallOpParam prm;
            prm.statusCode = PJSIP_SC_DECLINE;
            (*it)->hangup(prm);
        }
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
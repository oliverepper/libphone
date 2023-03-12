#ifndef ACCOUNT_T_H
#define ACCOUNT_T_H

#include "call_t.h"
#include <pjsua2.hpp>

class account_t : public pj::Account {
public:
    std::optional<std::function<void(int)>> on_incoming_call;
    std::optional<std::function<void(int, int)>> on_call_state;

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
        m_calls.back()->on_call_state = on_call_state;
        if (on_incoming_call.has_value()) on_incoming_call.value()(prm.callId);
    }

    void make_call(const std::string& uri) {
        m_calls.emplace_back(std::make_unique<call_t>(*this, [this](int id){ delete_call(id); } ));
        m_calls.back()->on_call_state = on_call_state;
        pj::CallOpParam prm{true};
        prm.opt.videoCount = 0;
        m_calls.back()->makeCall(uri, prm);
    }

    void answer_call(int call_id) {
        auto it = std::find_if(std::begin(m_calls), std::end(m_calls), [&call_id](const auto& element) {
            return static_cast<int>(*element) == call_id;
        });
        if (it != std::end(m_calls)) {
            pj::CallOpParam prm;
            prm.statusCode = PJSIP_SC_OK;
            (*it)->answer(prm);
        }
    }

    void hangup_call(int call_id) {
        auto it = std::find_if(std::begin(m_calls), std::end(m_calls), [&call_id](const auto& element) {
            return static_cast<int>(*element) == call_id;
        });
        if (it != std::end(m_calls)) {
            pj::CallOpParam prm;
            prm.statusCode = PJSIP_SC_DECLINE;
            (*it)->hangup(prm);
        }
    }

    void hangup_calls() noexcept {
        m_calls.clear();
    }

    void delete_call(int id) {
        PJ_LOG(3,(__BASE_FILE__, "Going to delete call: %d", id));
        auto it = std::find_if(std::begin(m_calls), std::end(m_calls), [&id](const auto& element) {
            return static_cast<int>(*element) == id;
        });
        if (it != std::end(m_calls)) { m_calls.erase(it); }
    }
private:
    std::vector<std::unique_ptr<call_t>> m_calls{};
} ;

#endif //ACCOUNT_T_H
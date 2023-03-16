#include "phone_instance_t.h"
#include "account_t.h"
#include <pjsua2.hpp>
#include <vector>

phone_instance_t::phone_instance_t(std::string user_agent, std::vector<std::string> nameserver,
                                   std::vector<std::string> stunserver) : m_ep{std::make_unique<pj::Endpoint>()},
                                                                          m_account{std::make_unique<account_t>()} {
    pj::EpConfig ep_cfg{};
    ep_cfg.uaConfig.userAgent = std::move(user_agent);
    ep_cfg.uaConfig.nameserver = std::move(nameserver);
    ep_cfg.uaConfig.stunServer = std::move(stunserver);
    try {
        m_ep->libCreate();
        m_ep->libInit(ep_cfg);
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

phone_instance_t::~phone_instance_t() {
    m_ep->libDestroy();
}

void phone_instance_t::register_on_call_state_callback(const std::function<void(int, int)>& callback) {
    m_account->on_call_state_with_index = callback;
}

void phone_instance_t::register_on_call_state_callback(const std::function<void(std::string, int)> &callback) {
    m_account->on_call_state_with_id = callback;
}

void phone_instance_t::register_on_incoming_call_callback(const std::function<void(int)>& callback) {
    m_account->on_incoming_call_with_index = callback;
}

void phone_instance_t::register_on_incoming_call_callback(const std::function<void(std::string)> &callback) {
    m_account->on_incoming_call_with_id = callback;
}

void phone_instance_t::configure_opus(int channel_count, int complexity, int sample_rate) {
    auto opus_cfg = m_ep->getCodecOpusConfig();
    opus_cfg.channel_cnt = channel_count;
    opus_cfg.complexity = complexity;
    opus_cfg.sample_rate = sample_rate;
    try {
        m_ep->setCodecOpusConfig(opus_cfg);
    } catch (const pj::Error &e) {
        throw phone::exception{e.info()};
    }
}

void phone_instance_t::create_tls_transport_with_srv_lookup() {
    pj::TransportConfig t_cfg;
    t_cfg.port = 0;
    try {
        m_ep->transportCreate(PJSIP_TRANSPORT_TLS, t_cfg);
    } catch (const pj::Error &e) {
        throw phone::exception{e.info()};
    }
}

void phone_instance_t::connect(std::string server, const std::string& user, std::optional<std::function<std::string()>> password) {
    m_server = std::move(server);
    pj::AuthCredInfo cred_info{};
    cred_info.realm = "*";
    cred_info.username = user;
    if (password.has_value()) cred_info.data = password.value()();

    pj::AccountConfig acc_cfg{};
    acc_cfg.mediaConfig.srtpUse = PJMEDIA_SRTP_OPTIONAL;
    acc_cfg.idUri = user + "<sip:" + user + "@" + m_server.value() + ">";
    acc_cfg.sipConfig.authCreds.push_back(cred_info);
    acc_cfg.regConfig.registrarUri = "sip:" + m_server.value() + ";transport=TLS";

    create_tls_transport_with_srv_lookup();

    try {
        m_ep->libStart();
        m_account->create(acc_cfg, true);
    } catch (const pj::Error &e) {
        throw phone::exception{e.info()};
    }
}

void phone_instance_t::make_call(const std::string& uri) {
    try {
        if (m_ep->utilVerifySipUri(uri) != PJ_SUCCESS) {
            m_account->make_call({"sip:" + uri + "@" + m_server.value() + ";transport=TLS"});
        } else {
            m_account->make_call(uri);
        }
    } catch (const pj::Error &e) {
        throw phone::exception{e.info()};
    } catch (const std::bad_optional_access& e) {
        throw phone::exception{e.what() + std::string{" – phone connected?"}};
    }
}

void phone_instance_t::answer_call(int id) {
    try {
        m_account->answer_call(id);
    } catch (const pj::Error &e) {
        throw phone::exception{e.info()};
    }
}

void PHONE_EXPORT phone_instance_t::answer_call(std::string call_id) {
    try {
        m_account->answer_call(std::move(call_id));
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

void phone_instance_t::hangup_call(int call_id) {
    try {
        m_account->hangup_call(call_id);
    } catch (const pj::Error &e) {
        throw phone::exception{e.info()};
    }
}

void PHONE_EXPORT phone_instance_t::hangup_call(std::string call_id) {
    try {
        m_account->hangup_call(std::move(call_id));
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

void phone_instance_t::hangup_calls() noexcept {
    m_account->hangup_calls();
}

void phone_instance_t::set_log_level(int level) {
    pj_log_set_level(level);
}

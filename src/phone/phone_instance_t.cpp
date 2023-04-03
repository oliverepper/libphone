#include "phone_instance_t.h"
#include "private/account_t.h"
#include <pjsua2.hpp>
#include <vector>

phone_instance_t::phone_instance_t(std::string user_agent, std::vector<std::string> nameserver,
                                   std::vector<std::string> stunserver) : m_ep{std::make_unique<pj::Endpoint>()},
                                                                          m_account{std::make_unique<account_t>()} {
    pj::EpConfig ep_cfg{};
    ep_cfg.uaConfig.userAgent = std::move(user_agent);
    ep_cfg.uaConfig.nameserver = std::move(nameserver);
    ep_cfg.uaConfig.stunServer = std::move(stunserver);

    ep_cfg.medConfig.ecOptions = PJMEDIA_ECHO_USE_SW_ECHO;
    try {
        m_ep->libCreate();
        m_ep->libInit(ep_cfg);
        m_ep->libStart();
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

void phone_instance_t::answer_call(std::string call_id) {
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

void phone_instance_t::hangup_call(std::string call_id) {
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

std::string phone_instance_t::get_call_id(int call_index) {
    try {
        return m_account->get_call_id(call_index);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

int phone_instance_t::get_call_index(const std::string& call_id) {
    try {
        return m_account->get_call_index(call_id);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

std::vector<phone::audio_device_info_t> phone_instance_t::get_audio_devices() {
    std::vector<phone::audio_device_info_t> result{pjmedia_aud_dev_count()};

    int index = 0;
    for (phone::audio_device_info_t& i : result) {
        pjmedia_aud_dev_info info;
        auto status = pjmedia_aud_dev_get_info(index, &info);
        if (status != PJ_SUCCESS) {
            char error_message[PJ_ERR_MSG_SIZE] = {0};
            pj_strerror(status, error_message, sizeof(error_message));
            throw phone::exception{error_message};
        } else {
            i.id = index;
            i.driver = info.driver;
            i.name = info.name;
            i.input_count = info.input_count;
            i.output_count = info.output_count;
        }
        ++index;
    }

    return result;
}

void phone_instance_t::set_audio_devices(int capture_index, int playback_index) {
    pjsua_snd_dev_param prm;
    pjsua_snd_dev_param_default(&prm);
    prm.capture_dev = capture_index;
    prm.playback_dev = playback_index;
    auto status = pjsua_set_snd_dev2(&prm);
    if (status != PJ_SUCCESS) {
        char error_message[PJ_ERR_MSG_SIZE] = {0};
        pj_strerror(status, error_message, sizeof(error_message));
        throw phone::exception{error_message};
    } else {
        PJ_LOG(3,(__BASE_FILE__, "did set capture device to: %d and playback device to: %d", capture_index, playback_index));
    }
}

void phone_instance_t::refresh_audio_devices() {
    pjmedia_aud_dev_refresh();
}

std::optional<std::string> phone_instance_t::call_incoming_message(int call_index) {
    try {
        return m_account->call_incoming_message(call_index);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

std::optional<std::string> phone_instance_t::call_incoming_message(const std::string& call_id) {
    try {
        return m_account->call_incoming_message(call_id);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

std::optional<int> phone_instance_t::call_answer_after(int call_index) {
    try {
        return m_account->call_answer_after(call_index);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

std::optional<int> phone_instance_t::call_answer_after(const std::string& call_id) {
    try {
        return m_account->call_answer_after(call_id);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

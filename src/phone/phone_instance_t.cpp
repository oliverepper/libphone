#include "phone_instance_t.h"
#include "private/account_t.h"
#include "private/system_nameserver.h"
#include "private/tone_generator_helper.h"
#include "private/log_writer_t.h"

#include <stunning.h>

#include <pjsua2.hpp>
#include <vector>
#include <iostream>

phone_instance_t::phone_instance_t(std::string user_agent,
                                   std::vector<std::string> nameserver,
                                   std::vector<std::string> stunserver)
: m_ep{std::make_unique<pj::Endpoint>()}, m_account{std::make_unique<account_t>()}, m_ep_cfg{std::make_unique<pj::EpConfig>()} {
    m_call_waiting_tone_generator = std::make_unique<pj::ToneGenerator>();
    m_dtmf_tone_generator = std::make_unique<pj::ToneGenerator>();


    m_ep_cfg->uaConfig.userAgent = std::move(user_agent);
    m_ep_cfg->uaConfig.nameserver = std::move(nameserver);
    m_ep_cfg->uaConfig.stunServer = std::move(stunserver);

    // FIXME: hopefully pjsip fixes the assumption about beeing the owner of the *log_writer_t
    // https://github.com/pjsip/pjproject/issues/3511
    m_log_writer = new log_writer_t{};
    m_ep_cfg->logConfig.writer = m_log_writer;

    m_ep_cfg->medConfig.ecOptions = PJMEDIA_ECHO_USE_SW_ECHO;

    try {
        m_ep->libCreate();
        m_ep->libInit(*m_ep_cfg);
        m_ep->audDevManager().setNullDev();
        m_ep->libStart();

        m_call_waiting_tone_generator->createToneGenerator();
        m_call_waiting_tone_generator->startTransmit2(m_ep->audDevManager().getPlaybackDevMedia(), {});

        m_dtmf_tone_generator->createToneGenerator();
        m_dtmf_tone_generator->startTransmit2(m_ep->audDevManager().getPlaybackDevMedia(), {});
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

phone_instance_t::phone_instance_t(std::string user_agent, std::vector<std::string> stunserver)
: phone_instance_t(std::move(user_agent), system_nameserver(), std::move(stunserver)) {}

phone_instance_t::~phone_instance_t() {
    m_dtmf_tone_generator.reset();
    m_call_waiting_tone_generator.reset();
    m_ep->libDestroy();
}

void phone_instance_t::register_on_registration_state_callback(const std::function<void(bool, int)> &callback) {
    m_account->on_registration_state = callback;
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

void create_tls_transport_with_srv_lookup(pj::Endpoint &endpoint) {
    pj::TransportConfig t_cfg;
    t_cfg.port = 0;
    try {
        endpoint.transportCreate(PJSIP_TRANSPORT_TLS, t_cfg);
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

    try {
        create_tls_transport_with_srv_lookup(*m_ep);
        if (m_account->isValid()) {
            m_account->modify(acc_cfg);
            m_account->setRegistration(true);
        }
        else
            m_account->create(acc_cfg, true);
    } catch (const pj::Error &e) {
        throw phone::exception{e.info()};
    }
}

void phone_instance_t::disconnect() {
    try {
        for (const auto &id: m_ep->transportEnum())
            m_ep->transportClose(id);
        if (m_account->isValid())
            m_account->setRegistration(false);
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
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

void phone_instance_t::answer_call(std::string call_id) {
    try {
        m_account->answer_call(std::move(call_id));
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

void phone_instance_t::start_ringing_call(int id) {
    try {
        m_account->start_ringing_call(id);
    } catch (const pj::Error &e) {
        throw phone::exception{e.info()};
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

void phone_instance_t::start_ringing_call(std::string call_id) {
    try {
        m_account->start_ringing_call(std::move(call_id));
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

void phone_instance_t::hangup_call(int call_id) {
    try {
        m_account->hangup_call(call_id);
    } catch (const pj::Error &e) {
        throw phone::exception{e.info()};
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

void phone_instance_t::hangup_call(std::string call_id) {
    try {
        m_account->hangup_call(std::move(call_id));
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

void phone_instance_t::dtmf(int call_index, const std::string &digits) const {
    try {
        m_account->dial_dtmf(call_index, digits);
        play_tones(*m_dtmf_tone_generator, digits);
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

void phone_instance_t::dtmf(std::string call_id, const std::string &digits) const {
    try {
        m_account->dial_dtmf(std::move(call_id), digits);
        play_tones(*m_dtmf_tone_generator, digits);
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

void phone_instance_t::hangup_calls() noexcept {
    m_account->hangup_calls();
}

void phone_instance_t::set_log_level(int level) {
    pj_log_set_level(level);
}

std::string phone_instance_t::get_call_id(int call_index) const {
    try {
        return m_account->get_call_id(call_index);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

int phone_instance_t::get_call_index(const std::string& call_id) const {
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

void phone_instance_t::set_audio_devices(int capture_index, int playback_index, bool use_global_sound_device_setting) {
    pjsua_snd_dev_param prm;
    pjsua_snd_dev_param_default(&prm);
    prm.capture_dev = capture_index;
    prm.playback_dev = playback_index;
    prm.use_default_settings = use_global_sound_device_setting;
    auto status = pjsua_set_snd_dev2(&prm);
    if (status != PJ_SUCCESS) {
        char error_message[PJ_ERR_MSG_SIZE] = {0};
        pj_strerror(status, error_message, sizeof(error_message));
        throw phone::exception{error_message};
    } else {
        PJ_LOG(3,(__FILE_NAME__, "did set capture device to: %d and playback device to: %d", prm.capture_dev, prm.playback_dev));
    }
}

void phone_instance_t::set_no_sound_devices() {
    pjsua_set_no_snd_dev();
}


void phone_instance_t::disconnect_audio_devices() {
    pjsua_set_no_snd_dev();
}

void phone_instance_t::refresh_audio_devices() {
    auto status = pjmedia_aud_dev_refresh();
    if (status != PJ_SUCCESS) {
        char error_message[PJ_ERR_MSG_SIZE] = {0};
        pj_strerror(status, error_message, sizeof(error_message));
        throw phone::exception{error_message};
    }
}

std::optional<std::string> phone_instance_t::call_incoming_message(int call_index) const {
    try {
        return m_account->call_incoming_message(call_index);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

std::optional<std::string> phone_instance_t::call_incoming_message(const std::string& call_id) const {
    try {
        return m_account->call_incoming_message(call_id);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

std::optional<int> phone_instance_t::call_answer_after(int call_index) const {
    try {
        return m_account->call_answer_after(call_index);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

std::optional<int> phone_instance_t::call_answer_after(const std::string& call_id) const {
    try {
        return m_account->call_answer_after(call_id);
    } catch (const std::invalid_argument& e) {
        throw phone::exception{e.what()};
    }
}

void phone_instance_t::register_thread(const std::string &name) {
    try {
        m_ep->libRegisterThread(name);
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

bool phone_instance_t::is_thread_registered() const {
    return m_ep->libIsThreadRegistered();
}

void phone_instance_t::play_call_waiting() const {
    try {
        m_call_waiting_tone_generator->play(call_waiting_sequence(), true);
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

void phone_instance_t::stop_call_waiting() const {
    try {
        m_call_waiting_tone_generator->stop();
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

unsigned int phone_instance_t::get_call_count() {
    return m_account->get_call_count();
}

void phone_instance_t::set_log_function(const std::function<void(int, std::string_view message, long thread_id, std::string_view thread_name)>& log_function) {
    m_log_writer->log_function = [log_function](int level, std::string_view message, long thread_id, std::string_view thread_name){
        log_function(level, message, thread_id, thread_name);
    };
}

void phone_instance_t::handle_ip_change() {
    pjsua_ip_change_param prm;
    pjsua_ip_change_param_default(&prm);
    auto status = pjsua_handle_ip_change(&prm);
    if (status != PJ_SUCCESS) {
        char buffer[PJ_ERR_MSG_SIZE];
        pj_strerror(status, buffer, sizeof(buffer));
        throw phone::exception{buffer};
    }
}

void phone_instance_t::adjust_tx_level_for_capture_device(float level) const {
    try {
        adjust_level_for_capture_device(phone::tx_rx_direction::Transmit, level);
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

void phone_instance_t::adjust_rx_level_for_capture_device(float level) const {
    try {
        adjust_level_for_capture_device(phone::tx_rx_direction::Receive, level);
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

float phone_instance_t::get_tx_level_adjustment_for_capture_device() const {
    try {
        return get_level_adjustment_for_capture_device(phone::tx_rx_direction::Transmit);
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

float phone_instance_t::get_rx_level_adjustment_for_capture_device() const {
    try {
        return get_level_adjustment_for_capture_device(phone::tx_rx_direction::Receive);
    } catch (const pj::Error& e) {
        throw phone::exception{e.info()};
    }
}

float phone_instance_t::get_level_adjustment_for_capture_device(phone::tx_rx_direction direction) const {
    auto portInfo = m_ep->audDevManager().getCaptureDevMedia().getPortInfo();
    switch (direction) {
        case phone::tx_rx_direction::Transmit:
            return portInfo.txLevelAdj;
        case phone::tx_rx_direction::Receive:
            return portInfo.rxLevelAdj;
    }
}

void phone_instance_t::adjust_level_for_capture_device(phone::tx_rx_direction direction, float level) const {
    switch (direction) {
        case phone::tx_rx_direction::Transmit:
            m_ep->audDevManager().getCaptureDevMedia().adjustTxLevel(level);
            break;
        case phone::tx_rx_direction::Receive:
            m_ep->audDevManager().getCaptureDevMedia().adjustRxLevel(level);
            break;
    }
}

void phone_instance_t::crash() {
    std::cerr << "Terminating process from libphone because of user request" << std::endl;
    std::terminate();
}

std::string phone_instance_t::get_public_address() const {
    try {
        auto result = perform_binding_request(m_ep_cfg.get()->uaConfig.stunServer.front());
        if (result.has_value())
            return result->value;
        else
            throw phone::exception{"could not resolv public address"};
    } catch (const stunning::exception& e) {
        throw phone::exception{e.what()};
    }
}

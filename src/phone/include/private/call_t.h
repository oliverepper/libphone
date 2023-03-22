#ifndef CALL_T_H
#define CALL_T_H

#include <pjsua2.hpp>

class call_t : public pj::Call {
public:
    std::optional<std::function<void(int, int)>> on_call_state_with_index;
    std::optional<std::function<void(std::string, int)>> on_call_state_with_id;

    explicit call_t(pj::Account &account, std::function<void(int)> delete_from_account, int call_id = PJSUA_INVALID_ID)
            : pj::Call(account, call_id), m_delete_call_from_account{std::move(delete_from_account)} {}

    explicit operator int() { return getId(); }
    explicit operator std::string() { return getInfo().callIdString; }

    void onCallState(pj::OnCallStateParam &prm) override {
        auto info = getInfo();
        if (on_call_state_with_index.has_value()) on_call_state_with_index.value()(info.id, info.state);
        if (on_call_state_with_id.has_value()) on_call_state_with_id.value()(info.callIdString, info.state);
        if (info.state == PJSIP_INV_STATE_DISCONNECTED) {
            PJ_LOG(3, (__BASE_FILE__, "calling delete function for call: %d with id: %s", info.id, info.callIdString.c_str()));
            m_delete_call_from_account(info.id);
        }
    }

    void onCallMediaState(pj::OnCallMediaStateParam &prm) override {
        auto info = getInfo();
        for (const auto& media : info.media) {
            if (media.type == PJMEDIA_TYPE_AUDIO) {
                auto& manager = pj::Endpoint::instance().audDevManager();
                if (media.index <= static_cast<unsigned int>(std::numeric_limits<int>::max())) {
                    auto audio_media = getAudioMedia(static_cast<int>(media.index));
                    audio_media.startTransmit(manager.getPlaybackDevMedia());
                    manager.getCaptureDevMedia().startTransmit(audio_media);
                } else {
                    throw phone::exception{"media index out of range"};
                }
            }
        }
    }
private:
    std::function<void(int)> m_delete_call_from_account;
};

#endif //CALL_T_H
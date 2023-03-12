#ifndef CALL_T_H
#define CALL_T_H

#include <pjsua2.hpp>

class call_t : public pj::Call {
public:
    std::optional<std::function<void(int, int)>> on_call_state;

    explicit call_t(pj::Account &account, std::function<void(int)> delete_from_account, int call_id = PJSUA_INVALID_ID)
            : pj::Call(account, call_id), m_delete_call_from_account{std::move(delete_from_account)} {}

    explicit operator int() { return getId(); }

    void onCallState(pj::OnCallStateParam &prm) override {
        auto info = getInfo();
        if (on_call_state.has_value()) on_call_state.value()(info.id, info.state);
        if (info.state == PJSIP_INV_STATE_DISCONNECTED) {
            PJ_LOG(3, (__BASE_FILE__, "calling delete function for call: %d", info.id));
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
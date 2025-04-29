#ifndef ACCOUNT_T_H
#define ACCOUNT_T_H

#include "call_t.h"
#include "calls_t.h"
#include "pj/log.h"
#include "pjmedia/types.h"
#include "pjsua-lib/pjsua.h"
#include "pjsua2/call.hpp"
#include <cstring>
#include <phone.h>
#include <phone_instance_t.h>
#include <concepts>
#include <pjsua2.hpp>

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
      PJ_LOG(3, (__FILE__, "register code: %d", prm.code));
    } else {
      PJ_LOG(3, (__FILE__, "unregister code: %d", prm.code));
    }
    if (on_registration_state.has_value()) on_registration_state.value()(info.regIsActive, prm.code);
  }

  void onIncomingCall(pj::OnIncomingCallParam &prm) override {
    PJ_LOG(3, (__FILE__, "Incoming call: %d", prm.callId));
    auto current_call = m_calls.emplace(*this, [this](int id){ delete_call(id); }, prm.callId);

    auto rx_data = static_cast<pjsip_rx_data *>(prm.rdata.pjRxData);
    if (rx_data) {
      // as an escape hatch save the complete INVITE message as a string
      current_call->incoming_message.emplace(rx_data->msg_info.msg_buf);

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
            // REVISIT (oepper) : 1 is not necessarily right, the number may have more digits
            std::string number(++pos, 1);
            current_call->answer_after.emplace(std::stoi(number));
          } catch (const std::invalid_argument &e) {
            PJ_LOG(1, (__FILE__, "%s", e.what()));
          } catch (const std::out_of_range &e) {
            PJ_LOG(1, (__FILE__, "%s", e.what()));
          }
        } while (false);
        if (current_call->answer_after.has_value())
          break;
        call_info_header = static_cast<pjsip_generic_string_hdr *>(pjsip_msg_find_hdr_by_name(
                                                                                              rx_data->msg_info.msg, &call_info_key, call_info_header->next));
      }
    }

    current_call->on_call_state_with_index = on_call_state_with_index;
    current_call->on_call_state_with_id = on_call_state_with_id;

    if (on_incoming_call_with_index.has_value()) {
      auto index = static_cast<int>(*current_call);
      PJ_LOG(5, (__FILE__, "calling on_incoming_call with index: %d", index));
      on_incoming_call_with_index.value()(index);
    }
    if (on_incoming_call_with_id.has_value()) {
      auto id = static_cast<std::string>(*current_call);
      PJ_LOG(5, (__FILE__, "calling on_incoming_call with id: %s", id.c_str()));
      on_incoming_call_with_id.value()(id);
    }
  }

  void make_call(const std::string &uri) {
    auto current_call = m_calls.emplace(*this, [this](int id) { delete_call(id); });
    current_call->on_call_state_with_index = on_call_state_with_index;
    current_call->on_call_state_with_id = on_call_state_with_id;
    pj::CallOpParam prm{true};
    prm.opt.videoCount = 0;
    current_call->makeCall(uri, prm);
  }

  void answer_call(phone::CallID auto id) {
    auto call = m_calls.find(id);
    pj::CallOpParam prm;
    prm.statusCode = PJSIP_SC_OK;
    call->answer(prm);
  }

  void start_ringing_call(phone::CallID auto id) {
    auto call = m_calls.find(id);
    pj::CallOpParam prm;
    prm.statusCode = PJSIP_SC_RINGING;
    call->answer(prm);
  }

  void hangup_call(phone::CallID auto id) {
    auto call = m_calls.find(id);
    pj::CallOpParam prm;
    prm.statusCode = PJSIP_SC_DECLINE;
    call->hangup(prm);
  }

  void dial_dtmf(phone::CallID auto id, const std::string& digits) {
    auto call = m_calls.find(id);
    call->dialDtmf(digits);
  }

  std::optional<std::string> call_incoming_message(phone::CallID auto id) {
    auto call = m_calls.find(id);
    return call->incoming_message;
  }

  inline rtcpstreamstat_t convertRtcpStreamStat(const pj::RtcpStreamStat &in) {
    return {
      .update = {.sec = in.update.sec, .msec = in.update.msec},
      .updateCount = in.updateCount, .pkt = in.pkt, .bytes = in.bytes,
      .discard = in.discard, .loss = in.loss, .reorder = in.reorder,
      .dup = in.dup,
      .lossPeriodUsec = {.samples = in.lossPeriodUsec.n,
                         .max = in.lossPeriodUsec.max,
                         .min = in.lossPeriodUsec.min,
                         .last = in.lossPeriodUsec.last,
                         .mean = in.lossPeriodUsec.mean},
      .lossType = {.burst = in.lossType.burst, .random = in.lossType.random},
      .jitterUsec = {.samples = in.jitterUsec.n,
                     .max = in.jitterUsec.max,
                     .min = in.jitterUsec.min,
                     .last = in.jitterUsec.last,
                     .mean = in.jitterUsec.mean}
    };
  }

  inline mathstat_t convertMathStat(const pj::MathStat &in) {
    return {
      .samples = in.n, .max = in.max, .min = in.min,
      .last = in.last, .mean = in.mean
    };
  }

  std::vector<rtcpstat_t> call_stats(phone::CallID auto id) {
    auto call = m_calls.find(id);
    pj::CallInfo ci = call->getInfo();
    pj::CallMediaInfoVector mediaInfo = ci.media;
    std::vector<rtcpstat_t> stats{};
    for (unsigned i = 0; i < mediaInfo.size(); ++i) {
      if (mediaInfo[i].type == PJMEDIA_TYPE_AUDIO &&
          mediaInfo[i].status == PJSUA_CALL_MEDIA_ACTIVE) {
        pj::StreamStat stat = call->getStreamStat(i);
        rtcpstat_t rtcpStat{};
        rtcpStat.rxStat = convertRtcpStreamStat(stat.rtcp.rxStat);
        rtcpStat.txStat = convertRtcpStreamStat(stat.rtcp.txStat);
        rtcpStat.rttUsec = convertMathStat(stat.rtcp.rttUsec);
        stats.emplace_back(std::move(rtcpStat));
      }
    }

    return stats;
  }

  std::optional<int> call_answer_after(phone::CallID auto id) {
    auto call = m_calls.find(id);
    return call->answer_after;
  }

  std::string get_call_id(int call_index) {
    auto call = m_calls.find(call_index);
    return static_cast<std::string>(*call);
  }

  int get_call_index(const std::string &call_id) {
    auto call = m_calls.find(call_id);
    return static_cast<int>(*call);
  }

  size_t get_call_count() {
    return m_calls.size();
  }

  void delete_call(int call_index) {
    PJ_LOG(3, (__FILE__, "Going to delete call: %d", call_index));
    m_calls.erase(call_index);
  }

  void hangup_calls() noexcept {
    m_calls.clear();
  }
    
private:
  calls_t m_calls;
};

#endif //ACCOUNT_T_H

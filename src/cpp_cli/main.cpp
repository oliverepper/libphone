#include "simple_task_system.h"
#include <phone_instance_t.h>
#include <phone/version.hpp>
#include <iostream>
#include <thread>
#include <cassert>

[[maybe_unused]] auto password_function = []() { return std::string{PASSWORD}; };

struct app_state {
    phone_instance_t phone;
    std::string last_call_id;
    int last_call_index = -1;
    simple_task_system task_system{&phone};

    [[maybe_unused]] static void on_registration_state(bool is_registered, int registration_state) {
        if (is_registered) {
            std::cout << "phone registered: " << phone::status_name(registration_state) << std::endl;
        } else {
            std::cout << "phone unregistered: " << phone::status_name(registration_state) << std::endl;
        }
    }

    void on_incoming_call_cb(int call_index) {
        last_call_index = call_index;
        std::cout << std::this_thread::get_id()
                  << " – Incoming call index: " << last_call_index
                  << ", call id: " << phone.get_call_id(last_call_index)
                  << std::endl;

        auto incoming_message = phone.call_incoming_message(call_index);
        if (incoming_message.has_value())
            std::cout << incoming_message.value().substr(0, 64) + "... " << std::endl;

        auto answer_after = phone.call_answer_after(call_index);
        if (answer_after.has_value()) {
            std::cout << "Will auto-answer call for you" << std::endl;
            task_system.async([this, answer_after, call_index](){
                assert(phone.is_thread_registered() == true);
                std::this_thread::sleep_for(std::chrono::seconds(answer_after.value()));
                try {
                    phone.answer_call(call_index);
                } catch (const phone::exception& e) {
                    std::cerr << "In callback: " << e.what() << std::endl;
                }
            });
        } else {
            phone.start_ringing_call(call_index);
        }
    }

    void on_incoming_call_cb(const std::string& call_id) {
        last_call_id = call_id;
        std::cout << std::this_thread::get_id()
                  << " - Incoming call id: " << last_call_id
                  << ", call index: " << phone.get_call_index(last_call_id)
                  << std::endl;

        auto incoming_message = phone.call_incoming_message(call_id);
        if (incoming_message.has_value())
            std::cout << incoming_message.value().substr(0, 64) + "... " << std::endl;

        auto answer_after = phone.call_answer_after(call_id);
        std::cout << "Will auto-answer call for you" << std::endl;
        if (answer_after.has_value()) {
            task_system.async([this, answer_after, call_id]() {
                assert(phone.is_thread_registered() == true);
                std::this_thread::sleep_for(std::chrono::seconds(answer_after.value()));
                try {
                    phone.answer_call(call_id);
                } catch (const phone::exception& e) {
                    std::cerr << "In callback: " << e.what() << std::endl;
                }
            });
        } else {
            phone.start_ringing_call(call_id);
        }
    }

    void on_call_state_with_index_cb(int call_index, int state) {
        last_call_index = call_index;
        std::cout << "Update for call index: " << call_index << " – state: " << phone::call_state_name(state) << std::endl;
        if (phone::call_state_name(state) == "DISCONNECTED") {
            last_call_index = -1;
        }
    }

    void on_call_state_with_id_cb(const std::string& call_id, int state) {
        last_call_id = call_id;
        std::cout << "Update for call id: " << call_id << " – state: " << phone::call_state_name(state) << std::endl;
        if (phone::call_state_name(state) == "DISCONNECTED") {
            last_call_id = "";
        }
    }
};

auto main() -> int {
    try {
        app_state state{
                phone_instance_t{
                    "Cli Phone in C++",
                    {"stun.t-online.de"}
                }};

        // set log level
        phone_instance_t::set_log_level(0);

        // and log function
        state.phone.set_log_function([](int level, std::string_view message, long thread_id, std::string_view thread_name){
            std::cout << message;
        });

        // callbacks
        state.phone.register_on_registration_state_callback([](bool is_registered, int registration_state) {
            app_state::on_registration_state(is_registered, registration_state);
        });
        state.phone.register_on_incoming_call_callback([&state](int call_index) {
            state.on_incoming_call_cb(call_index);
        });
        state.phone.register_on_incoming_call_callback([&state](const std::string& call_id) {
            state.on_incoming_call_cb(call_id);
        });
        state.phone.register_on_call_state_callback([&state](int call_index, int call_state) {
            state.on_call_state_with_index_cb(call_index, call_state);
        });
        state.phone.register_on_call_state_callback([&state](const std::string& call_id, int call_state) {
            state.on_call_state_with_id_cb(call_id, call_state);
        });

        // opus
        state.phone.configure_opus();

        // connect
        state.phone.connect(SERVER, USER, PASSWORD_FUNCTION);

        // repl
        char command = 'q';
        std::cout << "libphone version: " << phone_version_major() << "."
                                          << phone_version_minor() << "."
                                          << phone_version_patch() << std::endl;
        std::cout << phone_git_hash() << std::endl;
        std::cout << phone_git_description() << std::endl;
        std::cout << std::endl;

        do {
            try {
                std::cin >> command;
                if (command == 'c') {
                    std::string number;
                    std::cout << "please enter number: ";
                    std::cin >> number;
                    state.phone.make_call(number);
                } else if (command == 'C') {
                    state.phone.make_call(BUDDY_NUMBER);
                } else if (command == 'a') {
                    int call_index;
                    std::cout << "please enter call index: ";
                    std::cin >> call_index;
                    state.phone.answer_call(call_index);
                } else if (command == 'A') {
                    std::string call_id;
                    std::cout << "please enter call id: ";
                    std::cin >> call_id;
                    state.phone.answer_call(call_id);
                } else if (command == 'h') {
                    int call_index;
                    std::cout << "please enter call index: ";
                    std::cin >> call_index;
                    state.phone.hangup_call(call_index);
                } else if (command == 'H') {
                    std::string call_id;
                    std::cout << "please enter call id: ";
                    std::cin >> call_id;
                    state.phone.hangup_call(call_id);
                } else if (command == 'e') {
                    state.phone.hangup_calls();
                } else if (command == 'l') {
                    int level;
                    std::cout << "please enter desired log level 0..6: ";
                    std::cin >> level;
                    phone_instance_t::set_log_level(level);
                } else if (command == 'd') {
                    phone_instance_t::refresh_audio_devices();
                    for (const auto &e: phone_instance_t::get_audio_devices()) {
                        std::cout << e.id << " - " << e.driver << "/" << e.name << " (" << e.input_count << "/"
                                  << e.output_count << ")" << std::endl;
                    }
                } else if (command == 'D') {
                    int capture_index;
                    int playback_index;
                    std::cout << "please enter desired capture device: ";
                    std::cin >> capture_index;
                    std::cout << "please enter desired playback device: ";
                    std::cin >> playback_index;
                    phone_instance_t::set_audio_devices(capture_index, playback_index);
                } else if (command == 'S') {
                    std::cout << "Disconnecting audio devices from bridge" << std::endl;
                    phone_instance_t::disconnect_audio_devices();
                } else if (command == 'p') {
                    std::cout << "please enter dtmf characters: ";
                    std::string dtmf_digits;
                    std::cin >> dtmf_digits;
                    if (state.last_call_index != -1) {
                        state.phone.dtmf(state.last_call_index, dtmf_digits);
                    }
                } else if (command == 'b') {
                    std::cout << "playing call waiting" << std::endl;
                    state.phone.play_call_waiting();
                } else if (command == 'B') {
                    std::cout << "stop call waiting" << std::endl;
                    state.phone.stop_call_waiting();
                } else if (command == '#') {
                    std::cout << "call count: " << state.phone.get_call_count() << std::endl;
                } else if (command == 'i') {
                    std::cout << "handle ip change" << std::endl;
                    phone_instance_t::handle_ip_change();
                } else if (command == 't') {
                    std::cout << "TX level: " << state.phone.get_tx_level_adjustment_for_capture_device() << std::endl;
                } else if (command == 'r') {
                    std::cout << "RX level: " << state.phone.get_rx_level_adjustment_for_capture_device() << std::endl;
                } else if (command == 'm') {
                    state.phone.adjust_tx_level_for_capture_device(0);
                } else if (command == 'M') {
                    state.phone.adjust_tx_level_for_capture_device(1);
                } else if (command == '!') {
                    state.phone.crash();
                } else if (command == '0') {
                    state.phone.disconnect();
                } else if (command == '9') {
                    state.phone.connect(SERVER, USER, PASSWORD_FUNCTION);
                } else if (command == '6') {
                    for (const auto& address : state.phone.get_local_addresses())
                        std::cout << address << std::endl;
                }
            } catch (const phone::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        } while (command != 'q');
    } catch (const phone::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
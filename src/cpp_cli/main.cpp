#include <phone_instance_t.h>
#include <iostream>
#include <thread>
#include <fmt/core.h>

struct app_state {
    phone_instance_t phone;
    std::string last_call_id;
    int last_call_index;

    void on_incoming_call_cb(int call_index) {
        last_call_index = call_index;
        std::cout << std::this_thread::get_id()
                  << " – Incoming call index: " << last_call_index
                  << ", call id: " << phone.get_call_id(last_call_index)
                  << std::endl;
    }

    void on_incoming_call_cb(std::string call_id) {
        last_call_id = std::move(call_id);
        std::cout << std::this_thread::get_id()
                  << " - Incoming call id: " << last_call_id
                  << ", call index: " << phone.get_call_index(last_call_id)
                  << std::endl;
    }

    [[maybe_unused]] static void on_call_state_with_index_cb(int call_index, int state) {
        std::cout << "Update for call index: " << call_index << " – state: " << phone::state_name(state) << std::endl;
    }

    [[maybe_unused]] static void on_call_state_with_id_cb(const std::string& call_id, int state) {
        std::cout << "Update for call id: " << call_id << " – state: " << phone::state_name(state) << std::endl;
    }
};

auto main() -> int {
    try {
        app_state state{
                phone_instance_t{"Cli Phone in C++", {"217.237.148.22", "217.237.150.51"}, {"stun.t-online.de"}}};
        phone_instance_t::set_log_level(0);

        // callbacks
        state.phone.register_on_incoming_call_callback([&state](int call_index) {
            state.on_incoming_call_cb(call_index);
        });
        state.phone.register_on_incoming_call_callback([&state](std::string call_id) {
            state.on_incoming_call_cb(std::move(call_id));
        });
        state.phone.register_on_call_state_callback(app_state::on_call_state_with_index_cb);
        state.phone.register_on_call_state_callback(app_state::on_call_state_with_id_cb);

        // opus
        state.phone.configure_opus();

        // connect
        state.phone.connect("tel.t-online.de", "+4965191899543");

        // repl
        char command = 'q';
        do {
            std::cin >> command;
            if (command == 'c') {
                std::string number;
                std::cout << "please enter number: ";
                std::cin >> number;
                state.phone.make_call(number);
            } else if (command == 'C') {
                state.phone.make_call("+491804100100");
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
                for (const auto& e : state.phone.get_audio_devices()) {
                    fmt::print("{} - {}/{}({}/{})\n", e.id, e.driver, e.name, e.input_count, e.output_count);
                }
            } else if (command == 'D') {
                int capture_index;
                int playback_index;
                std::cout << "please enter desired capture device: ";
                std::cin >> capture_index;
                std::cout << "please enter desired playback device: ";
                std::cin >> playback_index;
                state.phone.set_audio_devices(capture_index, playback_index);
            }
        } while (command != 'q');
    } catch (const phone::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
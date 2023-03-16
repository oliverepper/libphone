#include <phone_instance_t.h>
#include <iostream>
#include <thread>

struct app {
    std::string last_call_id;
    int last_call_index;

    void on_incoming_call_cb(int call_index) {
        last_call_index = call_index;
        std::cout << std::this_thread::get_id() << " – Incoming call index : " << last_call_index << std::endl;
    }

    void on_incoming_call_cb(std::string call_id) {
        last_call_id = std::move(call_id);
        std::cout << std::this_thread::get_id() << " - Incoming call id: " << last_call_id << std::endl;
    }

    [[maybe_unused]] static void on_call_state_with_index_cb(int call_index, int state) {
        std::cout << "Update for call index: " << call_index << " – state: " << phone::state_name(state) << std::endl;
    }

    [[maybe_unused]] static void on_call_state_with_id_cb(const std::string& call_id, int state) {
        std::cout << "Update for call id: " << call_id << " – state: " << phone::state_name(state) << std::endl;
    }
};

auto main() -> int {
    app a{};
    try {
        auto phone = phone_instance_t{"Cli Phone in C++", {"217.237.148.22", "217.237.150.51"}, {"stun.t-online.de"}};
        phone_instance_t::set_log_level(0);

        // callbacks
        phone.register_on_incoming_call_callback([&a](int call_index) {
            a.on_incoming_call_cb(call_index);
        });
        phone.register_on_incoming_call_callback([&a](std::string call_id) {
            a.on_incoming_call_cb(std::move(call_id));
        });
        phone.register_on_call_state_callback(app::on_call_state_with_index_cb);
        phone.register_on_call_state_callback(app::on_call_state_with_id_cb);

        // opus
        phone.configure_opus();

        // connect
        phone.connect("tel.t-online.de", "+4965191899543");

        // repl
        char command = 'q';
        do {
            std::cin >> command;
            if (command == 'c') {
                std::string number;
                std::cout << "please enter number: ";
                std::cin >> number;
                phone.make_call(number);
            } else if (command == 'C') {
                phone.make_call("+4915123595397");
            } else if (command == 'a') {
                int call_index;
                std::cout << "please enter call index: ";
                std::cin >> call_index;
                phone.answer_call(call_index);
            } else if (command == 'A') {
                std::string call_id;
                std::cout << "please enter call id: ";
                std::cin >> call_id;
                phone.answer_call(call_id);
            } else if (command == 'h') {
                int call_index;
                std::cout << "please enter call index: ";
                std::cin >> call_index;
                phone.hangup_call(call_index);
            } else if (command == 'H') {
                std::string call_id;
                std::cout << "please enter call id: ";
                std::cin >> call_id;
                phone.hangup_call(call_id);
            } else if (command == 'e') {
                phone.hangup_calls();
            } else if (command == 'l') {
                int level;
                std::cout << "please enter desired log level 0..6: ";
                std::cin >> level;
                phone_instance_t::set_log_level(level);
            }
        } while (command != 'q');
    } catch (const phone::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
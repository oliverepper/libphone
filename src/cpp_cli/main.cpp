#include <phone_instance_t.h>
#include <iostream>
#include <thread>

struct app {
    int last_id;
    void on_incoming_call_cb(int call_id) {
        last_id = call_id;
        std::cout << std::this_thread::get_id() << " – Incoming call: " << last_id << std::endl;
    }

    [[maybe_unused]] static void on_call_state_cb(int call_id, int state) {
        std::cout << "Call: " << call_id << " – state: " << phone::state_name(state) << std::endl;
    }
};

auto main() -> int {
    app a{};
    try {
        auto phone = phone_instance_t{"Cli Phone in C++", {"217.237.148.22", "217.237.150.51"}, {"stun.t-online.de"}};
        phone_instance_t::set_log_level(0);
        phone.register_on_incoming_call_callback([&a](int call_id) {
            a.on_incoming_call_cb(call_id);
        });
        phone.register_on_call_state_callback(app::on_call_state_cb);
        phone.configure_opus();
        phone.connect("tel.t-online.de", "+4965191899543");
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
                int call_id;
                std::cout << "please enter call id: ";
                std::cin >> call_id;
                phone.answer_call(call_id);
            } else if (command == 'h') {
                int call_id;
                std::cout << "please enter call id: ";
                std::cin >> call_id;
                phone.hangup_call(call_id);
            } else if (command == 'H') {
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
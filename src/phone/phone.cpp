#include <pjsua.h>
#include <phone.h>
#include <phone_instance_t.h>
#include <vector>
#include <cstring>

char global_last_error[1024] = "no error";

phone_t phone_create(const char *user_agent,
                     const char * const nameserver[], size_t nameserver_count,
                     const char * const stunserver[], size_t stunserver_count) {
    const std::vector<std::string> _nameserver(nameserver, nameserver + nameserver_count);
    const std::vector<std::string> _stunserver(stunserver, stunserver + stunserver_count);
    try {
        return new phone_instance_t{user_agent, _nameserver, _stunserver};
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return nullptr;
    }
}

void phone_register_on_call_state_callback(phone_t instance, void (*cb)(int, int, void *), void *ctx) {
    phone_register_on_call_state_index_callback(instance, cb, ctx);
}

void phone_register_on_call_state_index_callback(phone_t instance, void (*cb)(int, int, void *), void *ctx) {
    instance->register_on_call_state_callback([cb, ctx](int call_index, int state){
        cb(call_index, state, ctx);
    });
}

void phone_register_on_call_state_id_callback(phone_t instance, void (*cb)(const char *, int, void *), void *ctx) {
    instance->register_on_call_state_callback([cb, ctx](const std::string& call_id, int state){
       cb(call_id.c_str(), state, ctx);
    });
}

void phone_register_on_incoming_call_callback(phone_t instance, void (*cb)(int, void *), void *ctx) {
    phone_register_on_incoming_call_index_callback(instance, cb, ctx);
}

void phone_register_on_incoming_call_index_callback(phone_t instance, void (*cb)(int, void *), void *ctx) {
    instance->register_on_incoming_call_callback([cb, ctx](int call_index){
        cb(call_index, ctx);
    });
}

void phone_register_on_incoming_call_id_callback(phone_t instance, void (*cb)(const char *, void *), void *ctx) {
    instance->register_on_incoming_call_callback([cb, ctx](const std::string& call_id){
        cb(call_id.c_str(), ctx);
    });
}

void phone_set_log_level(int level) {
    phone_instance_t::set_log_level(level);
}

void phone_destroy(phone_t instance) {
    delete instance;
}

phone_status_t PHONE_EXPORT phone_configure_opus(phone_t instance, int channel_count, int complexity, int sample_rate) {
    try {
        instance->configure_opus(channel_count, complexity, sample_rate);
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_connect(phone_t instance, const char *server, const char *user, const char *password) {
    try {
        if (password != nullptr) {
            instance->connect(server, user, [&password]() { return password; });
        } else {
            instance->connect(server, user);
        }
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_make_call(phone_t instance, const char *uri) {
    try {
        instance->make_call(uri);
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_answer_call(phone_t instance, int call_id) {
    return phone_answer_call_index(instance, call_id);
}

phone_status_t phone_answer_call_index(phone_t instance, int call_index) {
    try {
        instance->answer_call(call_index);
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_answer_call_id(phone_t instance, const char *call_id) {
    try {
        instance->answer_call(call_id);
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return EXIT_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}


phone_status_t phone_hangup_call(phone_t instance, int call_id) {
    return phone_hangup_call_index(instance, call_id);
}

phone_status_t phone_hangup_call_index(phone_t instance, int call_index) {
    try {
        instance->hangup_call(call_index);
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_hangup_call_id(phone_t instance, const char *call_id) {
    try {
        instance->hangup_call(call_id);
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

void phone_hangup_calls(phone_t instance) {
    instance->hangup_calls();
}

phone_status_t phone_get_call_id(phone_t instance, int call_index, char *out, size_t size) {
    try {
        auto id = instance->get_call_id(call_index);
        strncpy(out, id.c_str(), size);
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_get_call_index(phone_t instance, const char *call_id, int *out) {
    try {
        *out = instance->get_call_index(call_id);
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

const char* phone_last_error(void) {
    return global_last_error;
}

void phone_state_name(char *buffer, size_t buffer_size, int state) {
    strncpy(buffer, std::string{phone::state_name(state)}.c_str(), buffer_size);
}

void phone_refresh_audio_devices(void) {
    phone_instance_t::refresh_audio_devices();
}

size_t phone_get_audio_devices_count(void) {
    return pjmedia_aud_dev_count();
}

size_t phone_get_audio_device_info_name_length(void) {
    return PJMEDIA_AUD_DEV_INFO_NAME_LEN;
}

phone_status_t phone_get_audio_device_names(char **device_names, size_t *devices_count, size_t max_device_name_length, device_filter_t filter) {
    std::function<bool(phone::audio_device_info_t)> pred;
    switch (filter) {
        case DEVICE_FILTER_INPUT:
            pred = [](const auto& info){ return info.input_count == 0; };
            break;
        case DEVICE_FILTER_OUTPUT:
            pred = [](const auto& info){ return info.output_count == 0; };
            break;
        default:
            pred = [](const auto& info){ return false; };
            break;
    }

    std::vector<phone::audio_device_info_t> devices{*devices_count};
    try {
        devices = phone_instance_t::get_audio_devices();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }

    devices.erase(std::remove_if(std::begin(devices), std::end(devices), pred), std::end(devices));

    int i = 0;
    for (const auto& e : devices) {
        if (i < *devices_count) {
            strncpy(device_names[i], e.name.c_str(), max_device_name_length);
            ++i;
        }
    }
    *devices_count = i;
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_set_audio_devices(int capture_device, int playback_device) {
    try {
        phone_instance_t::set_audio_devices(capture_device, playback_device);
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t
phone_get_audio_devices(audio_device_info_t *devices, size_t *devices_count, size_t max_device_name_length,
                        size_t max_driver_name_length) {
    // TODO: implement
    strncpy(global_last_error, "not yet implemented", sizeof(global_last_error));
    return PHONE_STATUS_FAILURE;
}

phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_index);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_id);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_index);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_id);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_index);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_id);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_index);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_id);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_index);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_id);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_index);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_id);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_index);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_id);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_index);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_id);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_index);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}

phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after) {
    try {
        auto call_answer_after = instance->call_answer_after(call_id);
        *answer_after = -1;
        if (call_answer_after.has_value())
            *answer_after = call_answer_after.value();
    } catch (const phone::exception& e) {
        strncpy(global_last_error, e.what(), sizeof(global_last_error));
        return PHONE_STATUS_FAILURE;
    }
    return PHONE_STATUS_SUCCESS;
}








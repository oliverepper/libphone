#!/usr/bin/env python3
import os
import sys
from ctypes import *

__current_file = os.path.abspath(__file__)
__current_dir = os.path.dirname(__current_file)

module = 'libphone'

locations = [
    "../lib",
    "../../cmake-build-debug/bin/phone"
]


class Found(Exception):
    def __init__(self, value):
        self.value = value


try:
    for location in locations:
        for extension in ['.dylib', '.so']:
            dir_path = os.path.join(__current_dir, location)
            file_path = os.path.join(dir_path, module + extension)
            if os.path.exists(file_path):
                raise Found(file_path)
    else:
        print("Cannot find required module, not much I can do. Bye!")
        exit(1)
except Found as f:
    try:
        libphone = CDLL(f.value)
    except Exception as e:
        print(e, "\n")
        library_path = os.path.normpath(os.path.join(__current_dir, '..', 'lib'))
        print(f"Please export LD_LIBRARY_PATH and add {library_path} to it")
        print(f"You can run LD_LIBRARY_PATH={library_path}:$LD_LIBRARY_PATH {sys.argv[0]}")
        exit(1)

# defines
PHONE_STATUS_SUCCESS = 0

# device_filter
device_filter_t = c_int
DEVICE_FILTER_NONE = 0
DEVICE_FILTER_INPUT = 1
DEVICE_FILTER_OUTPUT = 2


# PHONE_EXPORT phone_t phone_create(const char *user_agent,
#                                   const char * const nameserver[], size_t nameserver_count,
#                                   const char * const stunserver[], size_t stunserver_count);
def phone_create(user_agent, nameservers, stunservers):
    __phone_create = libphone.phone_create
    __phone_create.restype = c_void_p
    __phone_create.argtypes = [c_char_p, POINTER(c_char_p), c_size_t, POINTER(c_char_p), c_size_t]
    c_user_agent = c_char_p(user_agent.encode('utf-8'))

    c_nameservers = (c_char_p * len(nameservers))()
    for i in range(len(nameservers)):
        c_nameservers[i] = c_char_p(nameservers[i].encode('utf-8'))

    c_stunservers = (c_char_p * len(stunservers))()
    for i in range(len(stunservers)):
        c_stunservers[i] = c_char_p(stunservers[i].encode('utf-8'))

    return __phone_create(c_user_agent, c_nameservers, len(nameservers), c_stunservers, len(stunservers))


# PHONE_EXPORT void phone_destroy(phone_t instance);
phone_destroy = libphone.phone_destroy
phone_destroy.restype = None
phone_destroy.argtypes = [c_void_p]


# PHONE_EXPORT void phone_register_on_incoming_call_index_callback(phone_t instance, void (*cb)(int call_index, void *ctx), void *ctx);
phone_register_on_incoming_call_index_callback = libphone.phone_register_on_incoming_call_index_callback
phone_register_on_incoming_call_index_callback.restype = None
phone_register_on_incoming_call_index_callback.argtypes = [c_void_p, c_void_p, c_void_p]


# PHONE_EXPORT void phone_register_on_incoming_call_id_callback(phone_t instance, void (*cb)(const char *call_id, void *ctx), void *ctx);
phone_register_on_incoming_call_id_callback = libphone.phone_register_on_incoming_call_id_callback
phone_register_on_incoming_call_id_callback.restype = None
phone_register_on_incoming_call_id_callback.argtypes = [c_void_p, c_void_p, c_void_p]


# PHONE_DEPRECATED_EXPORT void phone_register_on_incoming_call_callback(phone_t instance, void (*cb)(int call_id, void *ctx), void *ctx);
phone_register_on_incoming_call_callback = phone_register_on_incoming_call_index_callback


# PHONE_EXPORT void phone_register_on_call_state_index_callback(phone_t instance, void (*cb)(int call_index, int call_state, void *ctx), void *ctx);
phone_register_on_call_state_index_callback = libphone.phone_register_on_call_state_index_callback
phone_register_on_call_state_index_callback.restype = None
phone_register_on_call_state_index_callback.argtypes = [c_void_p, c_void_p, c_void_p]


# PHONE_EXPORT void phone_register_on_call_state_id_callback(phone_t instance, void (*cb)(const char *call_id, int call_state, void *ctx), void *ctx);
phone_register_on_call_state_id_callback = libphone.phone_register_on_call_state_id_callback
phone_register_on_call_state_id_callback.restype = None
phone_register_on_call_state_id_callback.argtypes = [c_void_p, c_void_p, c_void_p]


# PHONE_DEPRECATED_EXPORT void phone_register_on_call_state_callback(phone_t instance, void (*cb)(int call_id, int call_state, void *ctx), void *ctx);
phone_register_on_call_state_callback = phone_register_on_call_state_index_callback


# PHONE_EXPORT phone_status_t phone_configure_opus(phone_t instance, int channel_count, int complexity, int sample_rate);
phone_configure_opus = libphone.phone_configure_opus
phone_configure_opus.restype = c_int
phone_configure_opus.argtypes = [c_void_p, c_int, c_int, c_int]


# PHONE_EXPORT phone_status_t phone_connect(phone_t instance, const char *server, const char *user, const char *password);
def phone_connect(phone, server, user, password=None):
    __phone_connect = libphone.phone_connect
    __phone_connect.restype = c_int
    __phone_connect.argtypes = [c_void_p, c_char_p, c_char_p, c_char_p]
    c_server = c_char_p(server.encode('utf-8'))
    c_user = c_char_p(user.encode('utf-8'))
    if password is not None:
        return __phone_connect(phone, c_server, c_user, c_char_p(password.encode('utf-8')))
    else:
        return __phone_connect(phone, c_server, c_user, None)


# PHONE_EXPORT phone_status_t phone_make_call(phone_t instance, const char *uri);
def phone_make_call(phone, uri):
    __phone_make_call = libphone.phone_make_call
    __phone_make_call.restype = c_int
    __phone_make_call.argtypes = [c_void_p, c_char_p]
    c_uri = c_char_p(uri.encode('utf-8'))
    return __phone_make_call(phone, c_uri)


# PHONE_EXPORT phone_status_t phone_answer_call_index(phone_t instance, int call_index);
__phone_answer_call_index = libphone.phone_answer_call_index
__phone_answer_call_index.restype = c_int
__phone_answer_call_index.argtypes = [c_void_p, c_int]


# PHONE_EXPORT phone_status_t phone_answer_call_id(phone_t instance, const char *call_id);
def phone_answer_call_id(phone, call_id):
    __phone_answer_call_id = libphone.phone_answer_call_id
    __phone_answer_call_id.restype = c_int
    __phone_answer_call_id.argtypes = [c_void_p, c_char_p]
    c_call_id = c_char_p(call_id.encode('utf-8'))
    return __phone_answer_call_id(phone, c_call_id)


# PHONE_DEPRECATED_EXPORT phone_status_t phone_answer_call(phone_t instance, int call_id);
phone_answer_call = __phone_answer_call_index


# PHONE_EXPORT phone_status_t phone_hangup_call_index(phone_t instance, int call_index);
__phone_hangup_call_index = libphone.phone_hangup_call
__phone_hangup_call_index.restype = c_int
__phone_hangup_call_index.argtypes = [c_void_p, c_int]


# PHONE_EXPORT phone_status_t phone_hangup_call_id(phone_t instance, const char *call_id);
def phone_hangup_call_id(phone, call_id):
    __phone_hangup_call_id = libphone.phone_hangup_call_id
    __phone_hangup_call_id.restype = c_int
    __phone_hangup_call_id.argtypes = [c_void_p, c_char_p]
    c_call_id = c_char_p(call_id.encode('utf-8'))
    return __phone_hangup_call_id(phone, c_call_id)


# PHONE_DEPRECATED_EXPORT phone_status_t phone_hangup_call(phone_t instance, int call_id);
phone_hangup_call = __phone_hangup_call_index


# PHONE_EXPORT void phone_hangup_calls(phone_t instance);
phone_hangup_calls = libphone.phone_hangup_calls
phone_hangup_calls.restype = None
phone_hangup_calls.argtypes = [c_void_p]


# PHONE_EXPORT phone_status_t phone_get_call_id(phone_t instance, int call_index, char *out, size_t size);
def phone_get_call_id(phone, call_index):
    __phone_get_call_id = libphone.phone_get_call_id
    __phone_get_call_id.restype = c_int
    __phone_get_call_id.argtypes = [c_void_p, c_int, c_char_p, c_size_t]
    buffer = create_string_buffer(128)
    if __phone_get_call_id(phone, call_index, buffer, len(buffer)) != PHONE_STATUS_SUCCESS:
        raise Exception(f"could not get call_id for index {call_index}")
    return buffer.value.decode()


# PHONE_EXPORT phone_status_t phone_get_call_index(phone_t instance, const char *call_id, int *out);
def phone_get_call_index(phone, call_id):
    __phone_get_call_index = libphone.phone_get_call_index
    __phone_get_call_index.restype = c_int
    __phone_get_call_index.argtypes = [c_void_p, c_char_p, POINTER(c_int)]
    index = c_int()
    if __phone_get_call_index(phone, call_id, byref(index)) != PHONE_STATUS_SUCCESS:
        raise Exception(f"could not get call_index for id {call_id.value.decode('utf-8')}")
    return index.value


# PHONE_EXPORT void phone_refresh_audio_devices();
phone_refresh_audio_devices = libphone.phone_refresh_audio_devices
phone_refresh_audio_devices.restype = None
phone_refresh_audio_devices.argtypes = None


# PHONE_EXPORT size_t phone_get_audio_devices_count();
phone_get_audio_devices_count = libphone.phone_get_audio_devices_count
phone_get_audio_devices_count.restype = c_size_t
phone_get_audio_devices_count.argtypes = None


# PHONE_EXPORT size_t phone_get_audio_device_info_name_length();
phone_get_audio_device_info_name_length = libphone.phone_get_audio_device_info_name_length
phone_get_audio_device_info_name_length.restype = c_size_t
phone_get_audio_device_info_name_length.argtypes = None


# PHONE_EXPORT phone_status_t phone_get_audio_device_names(char **device_names, size_t *devices_count, size_t max_device_name_length, device_filter_t filter);
def phone_get_audio_device_names(device_filter):
    __phone_get_audio_device_names = libphone.phone_get_audio_device_names
    __phone_get_audio_device_names.restype = c_int
    __phone_get_audio_device_names.argtypes = [POINTER(c_char_p), POINTER(c_size_t), c_size_t, device_filter_t]
    if not DEVICE_FILTER_NONE <= device_filter <= DEVICE_FILTER_OUTPUT:
        device_filter = DEVICE_FILTER_NONE
    c_count = c_size_t(phone_get_audio_devices_count())
    max_device_name_length = phone_get_audio_device_info_name_length()
    device_names = (c_char_p * c_count.value)()

    for i in range(c_count.value):
        device_names[i] = cast(create_string_buffer(max_device_name_length), c_char_p)

    if __phone_get_audio_device_names(device_names, byref(c_count), max_device_name_length, device_filter) != PHONE_STATUS_SUCCESS:
        return []

    return [device_name.decode('utf-8') for device_name in device_names[:c_count.value]]


# PHONE_EXPORT phone_status_t phone_set_audio_devices(int capture_device, int playback_device);
phone_set_audio_devices = libphone.phone_set_audio_devices
phone_set_audio_devices.restype = c_int
phone_set_audio_devices.argtypes = [c_int, c_int]


# PHONE_EXPORT const char* phone_last_error(void);
def phone_last_error():
    __phone_last_error = libphone.phone_last_error
    __phone_last_error.restype = c_char_p
    __phone_last_error.argtypes = None
    print("ERROR: " + __phone_last_error().decode('utf-8'))


# PHONE_EXPORT void phone_state_name(char *buffer, size_t buffer_size, int state);
def phone_state_name(state):
    __phone_state_name = libphone.phone_state_name
    __phone_state_name.restype = None
    __phone_state_name.argtypes = [c_char_p, c_size_t, c_int]
    buffer = create_string_buffer(64)
    __phone_state_name(buffer, len(buffer), state)
    return buffer.value.decode()


# PHONE_EXPORT void phone_set_log_level(int level);
phone_set_log_level = libphone.phone_set_log_level
phone_set_log_level.restype = None
phone_set_log_level.argtypes = [c_int]


# PHONE_EXPORT unsigned phone_version_major();
phone_version_major = libphone.phone_version_major
phone_version_major.restype = c_uint
phone_version_major.argtypes = None

# PHONE_EXPORT unsigned phone_version_minor();
phone_version_minor = libphone.phone_version_minor
phone_version_minor.restype = c_uint
phone_version_minor.argtypes = None

# PHONE_EXPORT unsigned phone_version_patch();
phone_version_patch = libphone.phone_version_patch
phone_version_patch.restype = c_uint
phone_version_patch.argtypes = None

# __attribute__((unused)) unsigned phone_version_tweak();
phone_version_tweak = libphone.phone_version_tweak
phone_version_tweak.restype = c_uint
phone_version_tweak.argtypes = None


# PHONE_EXPORT void phone_git_hash(char *out, size_t size);
def phone_git_hash():
    __phone_git_hash = libphone.phone_git_hash
    __phone_git_hash.restype = None
    __phone_git_hash.argtypes = [c_char_p, c_size_t]
    buffer = create_string_buffer(64)
    __phone_git_hash(buffer, len(buffer))
    return buffer.value.decode('utf-8')


# PHONE_EXPORT void phone_git_description(char *out, size_t size);
def phone_git_description():
    __phone_git_description = libphone.phone_git_description
    __phone_git_description.restype = None
    __phone_git_description.argtypes = [c_char_p, c_size_t]
    buffer = create_string_buffer(64)
    __phone_git_description(buffer, len(buffer))
    return buffer.value.decode('utf-8')


def die(instance):
    phone_destroy(instance)
    phone_last_error()
    exit(1)


helptext = '''
c - call a number
C - call Time Announcement of Telekom Germany
a - answer a call (index)
A - answer a call (id)
h - hangup a call (index)
H - hangup a call (id)
e - kill all calls
l - change log level
d - list audio devices
D - change audio devices
q - quit
'''

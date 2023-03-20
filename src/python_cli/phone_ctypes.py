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

# create_phone
__phone_create = libphone.phone_create
__phone_create.restype = c_void_p
__phone_create.argtypes = [c_char_p, POINTER(c_char_p), c_size_t, POINTER(c_char_p), c_size_t]

# phone_register_on_incoming_call_callback
phone_register_on_incoming_call_callback = libphone.phone_register_on_incoming_call_callback
phone_register_on_incoming_call_callback.restype = None
phone_register_on_incoming_call_callback.argtypes = [c_void_p, c_void_p, c_void_p]

# phone_register_on_call_state_callback
phone_register_on_call_state_callback = libphone.phone_register_on_call_state_callback
phone_register_on_call_state_callback.restype = None
phone_register_on_call_state_callback.argtypes = [c_void_p, c_void_p, c_void_p]

# phone_configure_opus
phone_configure_opus = libphone.phone_configure_opus
phone_configure_opus.restype = c_int
phone_configure_opus.argtypes = [c_void_p, c_int, c_int, c_int]

# phone_set_log_level
phone_set_log_level = libphone.phone_set_log_level
phone_set_log_level.restype = None
phone_set_log_level.argtypes = [c_int]

# phone_connect
__phone_connect = libphone.phone_connect
__phone_connect.restype = c_int
__phone_connect.argtypes = [c_void_p, c_char_p, c_char_p, c_char_p]

# phone_make_call
__phone_make_call = libphone.phone_make_call
__phone_make_call.restype = c_int
__phone_make_call.argtypes = [c_void_p, c_char_p]

# phone_answer_call
phone_answer_call = libphone.phone_answer_call
phone_answer_call.restype = c_int
phone_answer_call.argtypes = [c_void_p, c_int]

# phone_hangup_call
phone_hangup_call = libphone.phone_hangup_call
phone_hangup_call.restype = c_int
phone_hangup_call.argtypes = [c_void_p, c_int]

# phone_hangup_call
phone_hangup_calls = libphone.phone_hangup_calls
phone_hangup_calls.restype = None
phone_hangup_calls.argtypes = [c_void_p]

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

# phone_get_audio_device_names
__phone_get_audio_device_names = libphone.phone_get_audio_device_names
__phone_get_audio_device_names.restype = c_int
__phone_get_audio_device_names.argtypes = [POINTER(c_char_p), POINTER(c_size_t), c_size_t, c_int]

# set audio device
phone_set_audio_devices = libphone.phone_set_audio_devices
phone_set_audio_devices.restype = c_int
phone_set_audio_devices.argtypes = [c_int, c_int]

# destroy phone
phone_destroy = libphone.phone_destroy
phone_destroy.restype = None
phone_destroy.argtypes = [c_void_p]

# phone_state_name
__phone_state_name = libphone.phone_state_name
__phone_state_name.restype = None
__phone_state_name.argtypes = [c_char_p, c_size_t, c_int]

# last_error
__phone_last_error = libphone.phone_last_error
__phone_last_error.restype = c_char_p
__phone_last_error.argtypes = None


def phone_last_error():
    print("ERROR: " + __phone_last_error().decode('utf-8'))


def die(instance):
    phone_destroy(instance)
    phone_last_error()
    exit(1)


def phone_get_audio_device_names(filter):
    if not 0 <= filter <= 2:
        filter = 0
    c_count = c_size_t(phone_get_audio_devices_count())
    max_device_name_length = phone_get_audio_device_info_name_length()
    device_names = (c_char_p * c_count.value)()

    for i in range(c_count.value):
        device_names[i] = cast(create_string_buffer(max_device_name_length), c_char_p)

    if __phone_get_audio_device_names(device_names, byref(c_count), max_device_name_length, filter) != PHONE_STATUS_SUCCESS:
        return []

    return [e.decode('utf-8') for e in device_names[:c_count.value]]


def phone_create(user_agent, nameservers, stunservers):
    c_user_agent = c_char_p(user_agent.encode('utf-8'))

    c_nameservers = (c_char_p * len(nameservers))()
    for i in range(len(nameservers)):
        c_nameservers[i] = c_char_p(nameservers[i].encode('utf-8'))

    c_stunservers = (c_char_p * len(stunservers))()
    for i in range(len(stunservers)):
        c_stunservers[i] = c_char_p(stunservers[i].encode('utf-8'))

    return __phone_create(c_user_agent, c_nameservers, len(nameservers), c_stunservers, len(stunservers))


def phone_connect(phone, server, user, password=None):
    c_server = c_char_p(server.encode('utf-8'))
    c_user = c_char_p(user.encode('utf-8'))
    if password is not None:
        return __phone_connect(phone, c_server, c_user, c_char_p(password.encode('utf-8')))
    else:
        return __phone_connect(phone, c_server, c_user, None)


def phone_make_call(phone, uri):
    c_uri = c_char_p(uri.encode('utf-8'))
    return __phone_make_call(phone, c_uri)


def phone_state_name(state):
    buffer = create_string_buffer(64)
    __phone_state_name(buffer, len(buffer), state)
    return buffer.value.decode()


helptext = '''
c - call a number
C - call Time Announcement of Telekom Germany
a - answer a call
h - hangup a call
H - kill all calls
l - change log level
d - list audio devices
D - change audio devices
q - quit
'''

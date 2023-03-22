#!/usr/bin/env python3

import os
import sys

current_file = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file)
sys.path.insert(0, os.path.join(current_dir, '..', 'share'))

from phone_ctypes import *

useragent           = "Python CLI Phone"
nameservers         = ["217.237.148.22", "217.237.150.51"]
stunservers         = ["stun.t-online.de"]
sipserver           = "tel.t-online.de"
username            = "+4965191899543"
password            = None
opus_channel_count  = 1
opus_complexity     = 8
opus_sample_rate    = 16000
buddy               = "+491804100100"

if '...' in username:
    print(f"Username \"{username}\" invalid. Please update your phone configuration in {current_file}")
    exit(1)


phone = phone_create(useragent, nameservers, stunservers)
if phone is None:
    die(phone)
phone_set_log_level(0)


# callbacks
# noinspection PyShadowingNames,PyUnusedLocal
@CFUNCTYPE(None, c_int, c_void_p)
def on_incoming_call_index_cb(call_index, ctx):
    print(f"Incoming call with index: {call_index} and id: {phone_get_call_id(phone, call_index)}")


@CFUNCTYPE(None, c_char_p, c_void_p)
def on_incoming_call_id_cb(call_id, ctx):
    c_faulty_string = c_char_p('🥷'.encode('utf-8'))
    print(f"Incoming call with id: {call_id.decode('utf-8')} and index: {phone_get_call_index(phone, call_id)}")


# noinspection PyShadowingNames,PyUnusedLocal
@CFUNCTYPE(None, c_int, c_int, c_void_p)
def on_call_state_index_cb(call_index, state, ctx):
    print(f"Call index: {call_index} and id: {phone_get_call_id(phone, call_index)} – state: {phone_state_name(state)}")


# noinspection PyShadowingNames,PyUnusedLocal
@CFUNCTYPE(None, c_char_p, c_int, c_void_p)
def on_call_state_id_cb(call_id, state, ctx):
    print(f"Call id: {call_id.decode('utf-8')} and index: {phone_get_call_index(phone, call_id)} – state: {phone_state_name(state)}")


phone_register_on_incoming_call_index_callback(phone, on_incoming_call_index_cb, None)
phone_register_on_incoming_call_id_callback(phone, on_incoming_call_id_cb, None)
phone_register_on_call_state_index_callback(phone, on_call_state_index_cb, None)
phone_register_on_call_state_id_callback(phone, on_call_state_id_cb, None)


if phone_configure_opus(phone, opus_channel_count, opus_complexity, opus_sample_rate) != PHONE_STATUS_SUCCESS:
    die(phone)
if phone_connect(phone, sipserver, username, password) != PHONE_STATUS_SUCCESS:
    die(phone)
if phone_set_audio_devices(0, 1) != PHONE_STATUS_SUCCESS:
    die(phone)

print(helptext)

command = ''
while command != 'q':
    command = input()
    if command == 'C':
        if phone_make_call(phone, buddy) != PHONE_STATUS_SUCCESS:
            print(phone_last_error())
    elif command == 'c':
        number = input("please enter number: ")
        if phone_make_call(phone, number) != PHONE_STATUS_SUCCESS:
            print(phone_last_error())
    elif command == 'a':
        call_id = int(input("please enter call id: "))
        if phone_answer_call(phone, call_id) != PHONE_STATUS_SUCCESS:
            print(phone_last_error())
    elif command == 'A':
        call_id = input("please enter call id: ")
        if (phone_answer_call_id(phone, call_id)) != PHONE_STATUS_SUCCESS:
            print(phone_last_error())
    elif command == 'h':
        call_id = int(input("please enter call id: "))
        if phone_hangup_call(phone, call_id) != PHONE_STATUS_SUCCESS:
            print(phone_last_error())
    elif command == 'H':
        call_id = input("please enter call id: ")
        if phone_hangup_call_id(phone, call_id) != PHONE_STATUS_SUCCESS:
            print(phone_last_error())
    elif command == 'e':
        phone_hangup_calls(phone)
    elif command == 'l':
        level = int(input("please enter desired log-level 0..6: "))
        phone_set_log_level(level)
    elif command == 'd':
        phone_refresh_audio_devices()
        print("""
    0 - no filter
    1 - capture devices
    2 - playback devices
        """)
        try:
            device_filter = int(input("do you want a filter?: "))
        except ValueError:
            device_filter = DEVICE_FILTER_NONE
        for idx, device in enumerate(phone_get_audio_device_names(device_filter)):
            print(f"{idx} - {device}")
        print()
    elif command == 'D':
        capture_device = int(input("please enter desired capture device: "))
        playback_device = int(input("please enter desired playback device: "))
        if phone_set_audio_devices(capture_device, playback_device) != PHONE_STATUS_SUCCESS:
            print(phone_last_error())

print("shutting down...")
phone_destroy(phone)


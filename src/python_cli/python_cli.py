#!/usr/bin/env python3

import os
import sys
import time

current_file = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file)
sys.path.insert(0, os.path.join(current_dir, '..', 'share'))

from phone_ctypes import *

useragent           = "Python CLI Phone"
nameservers         = ["217.237.148.22", "217.237.150.51"]
stunservers         = ["stun.t-online.de"]
sipserver           = "tel.t-online.de"
username            = "+49..."
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

    answer_after = phone_call_answer_after_index(phone, call_index)
    if answer_after >= 0:
        # FIXME: push this in another thread, need phone_register_thread, first.
        print(f"will auto answer call after {answer_after} seconds")
        time.sleep(answer_after)
        phone_answer_call(phone, call_index)


@CFUNCTYPE(None, c_char_p, c_void_p)
def on_incoming_call_id_cb(call_id, ctx):
    c_faulty_string = c_char_p('🥷'.encode('utf-8'))
    print(f"Incoming call with id: {call_id.decode('utf-8')} and index: {phone_get_call_index(phone, call_id)}")

    answer_after = phone_call_answer_after_id(phone, call_id)
    if answer_after >= 0:
        # FIXME: push this in another thread, need phone_register_thread, first.
        print(f"will auto answer call after {answer_after} seconds")
        time.sleep(answer_after)
        phone_answer_call_id(phone, call_id.decode('utf-8'))


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
    print(phone_last_error(), file=sys.stderr)

print(helptext)

command = ''
while command != 'q':
    print(f"libphone version {phone_version_major()}.{phone_version_minor()}.{phone_version_patch()}-{phone_version_tweak()}")
    print(phone_git_hash())
    print(phone_git_description())
    print()

    command = input()
    if command == 'C':
        if phone_make_call(phone, buddy) != PHONE_STATUS_SUCCESS:
            print(phone_last_error(), file=sys.stderr)
    elif command == 'c':
        number = input("please enter number: ")
        if phone_make_call(phone, number) != PHONE_STATUS_SUCCESS:
            print(phone_last_error(), file=sys.stderr)
    elif command == 'a':
        call_id = int(input("please enter call id: "))
        if phone_answer_call(phone, call_id) != PHONE_STATUS_SUCCESS:
            print(phone_last_error(), file=sys.stderr)
    elif command == 'A':
        call_id = input("please enter call id: ")
        if (phone_answer_call_id(phone, call_id)) != PHONE_STATUS_SUCCESS:
            print(phone_last_error(), file=sys.stderr)
    elif command == 'h':
        call_id = int(input("please enter call id: "))
        if phone_hangup_call(phone, call_id) != PHONE_STATUS_SUCCESS:
            print(phone_last_error(), file=sys.stderr)
    elif command == 'H':
        call_id = input("please enter call id: ")
        if phone_hangup_call_id(phone, call_id) != PHONE_STATUS_SUCCESS:
            print(phone_last_error(), file=sys.stderr)
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
            print("no valid input – will use DEVICE_FILTER_NONE", file=sys.stderr)
            device_filter = DEVICE_FILTER_NONE
        for device in phone_get_audio_devices(device_filter):
            print(f"{device.id} - {device.driver}/{device.name} ({device.input_count}/{device.output_count})")
        print()
    elif command == 'D':
        capture_device = int(input("please enter desired capture device: "))
        playback_device = int(input("please enter desired playback device: "))
        if phone_set_audio_devices(capture_device, playback_device) != PHONE_STATUS_SUCCESS:
            print(phone_last_error(), file=sys.stderr)

print("shutting down...")
phone_destroy(phone)


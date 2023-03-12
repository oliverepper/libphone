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
def on_incoming_call(call_id, ctx):
    print(f"Incoming call with ID: {call_id}")


# noinspection PyShadowingNames,PyUnusedLocal
@CFUNCTYPE(None, c_int, c_int, c_void_p)
def on_call_state(call_id, state, ctx):
    print(f"Call: {call_id} – state: {phone_state_name(state)}")


phone_register_on_incoming_call_callback(phone, on_incoming_call, None)
phone_register_on_call_state_callback(phone, on_call_state, None)

if phone_configure_opus(phone, opus_channel_count, opus_complexity, opus_sample_rate) != PHONE_STATUS_SUCCESS:
    die(phone)
if phone_connect(phone, sipserver, username, password) != PHONE_STATUS_SUCCESS:
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
    elif command == 'h':
        call_id = int(input("please enter call id: "))
        if phone_hangup_call(phone, call_id) != PHONE_STATUS_SUCCESS:
            print(phone_last_error())
    elif command == 'H':
        phone_hangup_calls(phone)
    elif command == 'l':
        level = int(input("please enter desired log-level 0..6: "))
        phone_set_log_level(level)

print("shutting down...")
phone_destroy(phone)


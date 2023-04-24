package de.oliver_epper.libphone;

import com.sun.jna.*;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.LongByReference;

import java.util.Arrays;
import java.util.List;

public class Phone {
    static final int PHONE_STATUS_SUCCESS=0;
    static final int PHONE_STATUS_FAILURE=1;

    public record AudioDeviceInfo(int id, String driverName, String name, int inputCount, int outputCount) {

        @Override
        public String toString() {
            return id + " - " + driverName + "/" + name + " (" + inputCount + "/" + outputCount + ")";
        }
    }

    enum DeviceFilter {
        NONE(0),
        INPUT_DEVICES(1),
        OUPUT_DEVICES(2);

        final int val;

        DeviceFilter(int val) {
            this.val = val;
        }

        public int getIntValue() {
            return val;
        }
    }

    private interface PhoneLibrary extends Library {
        interface RegistrationStateCallback extends Callback {
            void onRegistrationState(boolean isRegistered, int registrationState, Pointer ctx);
        }

        interface IncomingCallIndexCallback extends Callback {
            void onIncomingCallIndexCallback(int callIndex, Pointer ctx);
        }

        interface IncomingCallIdCallback extends Callback {
            void onIncomingCallIdCallback(String callId, Pointer ctx);
        }

        interface CallStateIndexCallback extends Callback {
            void onCallStateIndexCallback(int callIndex, int state, Pointer ctx);
        }

        interface CallStateIdCallback extends Callback {
            void onCallStateIdCallback(String callId, int state, Pointer ctx);
        }

        interface LogFunction extends Callback {
            void logFunction(int level, String message, long threadId, String threadName);
        }

        // PHONE_EXPORT phone_t phone_create(const char *user_agent, const char * const nameserver[], size_t nameserver_count, const char * const stunserver[], size_t stunserver_count);
        Pointer phone_create(String userAgent, String[] nameServers, NativeLong nameServerLength, String[] stunServers, NativeLong stunServerLength);

        // PHONE_EXPORT phone_t phone_create_with_system_nameserver(const char *user_agent, const char * const stunserver[], size_t stunserver_count);
        Pointer phone_create_with_system_nameserver(String userAgent, String[] stunServers, NativeLong stunServerLength);

        // PHONE_EXPORT void phone_destroy(phone_t instance);
        void phone_destroy(Pointer instance);

        // PHONE_EXPORT void phone_register_on_registration_state_callback(phone_t instance, void (*cb)(int is_registered, int registration_state, void *ctx), void *ctx);
        void phone_register_on_registration_state_callback(Pointer instance, RegistrationStateCallback cb, Pointer ctx);

        // PHONE_DEPRECATED_EXPORT void phone_register_on_incoming_call_callback(phone_t instance, void (*cb)(int call_id, void *ctx), void *ctx);
        // PHONE_EXPORT void phone_register_on_incoming_call_index_callback(phone_t instance, void (*cb)(int call_index, void *ctx), void *ctx);
        void phone_register_on_incoming_call_index_callback(Pointer phone, IncomingCallIndexCallback cb, Pointer ctx);
        // PHONE_EXPORT void phone_register_on_incoming_call_id_callback(phone_t instance, void (*cb)(const char *call_id, void *ctx), void *ctx);
        void phone_register_on_incoming_call_id_callback(Pointer phone, IncomingCallIdCallback cb, Pointer ctx);

        // PHONE_DEPRECATED_EXPORT void phone_register_on_call_state_callback(phone_t instance, void (*cb)(int call_id, int call_state, void *ctx), void *ctx);
        // PHONE_EXPORT void phone_register_on_call_state_index_callback(phone_t instance, void (*cb)(int call_index, int call_state, void *ctx), void *ctx);
        void phone_register_on_call_state_index_callback(Pointer instance, CallStateIndexCallback cb, Pointer ctx);
        // PHONE_EXPORT void phone_register_on_call_state_id_callback(phone_t instance, void (*cb)(const char *call_id, int call_state, void *ctx), void *ctx);
        void phone_register_on_call_state_id_callback(Pointer instance, CallStateIdCallback cb, Pointer ctx);

        // PHONE_EXPORT phone_status_t phone_configure_opus(phone_t instance, int channel_count, int complexity, int sample_rate);
        int phone_configure_opus(Pointer phone, int channel_count, int complexity, int sample_rate);

        // PHONE_EXPORT phone_status_t phone_connect(phone_t instance, const char *server, const char *user, const char *password);
        int phone_connect(Pointer phone, String server, String user, String password);

        // PHONE_EXPORT phone_status_t phone_make_call(phone_t instance, const char *uri);
        int phone_make_call(Pointer phone, String number);

        // PHONE_DEPRECATED_EXPORT phone_status_t phone_answer_call(phone_t instance, int call_id);
        // PHONE_EXPORT phone_status_t phone_answer_call_index(phone_t instance, int call_index);
        int phone_answer_call_index(Pointer phone, int call_index);
        // PHONE_EXPORT phone_status_t phone_answer_call_id(phone_t instance, const char *call_id);
        int phone_answer_call_id(Pointer phone, String call_id);

        // PHONE_EXPORT phone_status_t phone_start_ringing_call_index(phone_t instance, int call_index);
        int phone_start_ringing_call_index(Pointer instance, int callIndex);
        // PHONE_EXPORT phone_status_t phone_start_ringing_call_id(phone_t instance, const char *call_id);
        int phone_start_ringing_call_id(Pointer instance, String CallId);

        // PHONE_DEPRECATED_EXPORT phone_status_t phone_hangup_call(phone_t instance, int call_id);
        // PHONE_EXPORT phone_status_t phone_hangup_call_index(phone_t instance, int call_index);
        int phone_hangup_call_index(Pointer instance, int call_index);
        // PHONE_EXPORT phone_status_t phone_hangup_call_id(phone_t instance, const char *call_id);
        int phone_hangup_call_id(Pointer instance, String callId);

        // PHONE_EXPORT phone_status_t phone_play_dtmf_call_index(phone_t instance, int call_index, const char *digits);
        int phone_play_dtmf_call_index(Pointer instance, int call_index, String digits);
        // PHONE_EXPORT phone_status_t phone_play_dtmf_call_id(phone_t instance, const char *call_id, const char *digits);
        int phone_play_dtmf_call_id(Pointer instance, String call_id, String digits);

        // PHONE_EXPORT void phone_hangup_calls(phone_t instance);
        void phone_hangup_calls(Pointer instance);

        // PHONE_EXPORT phone_status_t phone_get_call_id(phone_t instance, int call_index, char *call_id, size_t size);
        int phone_get_call_id(Pointer phone, int call_index, byte[] call_id, NativeLong size);
        // PHONE_EXPORT phone_status_t phone_get_call_index(phone_t instance, const char *call_id, int *out);
        int phone_get_call_index(Pointer phone, String call_id, IntByReference index_ref);

        // PHONE_EXPORT void phone_refresh_audio_devices(void);
        void phone_refresh_audio_devices();
        // PHONE_EXPORT size_t phone_get_audio_devices_count(void);
        NativeLong phone_get_audio_devices_count();
        // PHONE_EXPORT size_t phone_get_audio_device_driver_name_length(void);
        NativeLong phone_get_audio_device_driver_name_length();
        // PHONE_EXPORT size_t phone_get_audio_device_info_name_length(void);
        NativeLong phone_get_audio_device_info_name_length();
        // PHONE_DEPRECATED_EXPORT phone_status_t phone_get_audio_device_names(char **device_names, size_t *devices_count, size_t max_device_name_length, device_filter_t filter);

        // PHONE_EXPORT phone_status_t phone_get_audio_devices(audio_device_info_t *devices, size_t *devices_count, size_t max_driver_name_length, size_t max_device_name_length, device_filter_t filter);
        int phone_get_audio_devices(AudioDeviceInfo[] devices, LongByReference devices_count, NativeLong max_driver_name_length, NativeLong max_device_name_length, int filter);
        // PHONE_EXPORT phone_status_t phone_set_audio_devices(int capture_device, int playback_device);
        int phone_set_audio_devices(int capture_device, int playback_device);

        // PHONE_EXPORT phone_status_t phone_call_answer_after_index(phone_t instance, int call_index, int *answer_after);
        int phone_call_answer_after_index(Pointer phone, int call_index, IntByReference answer_after);
        // PHONE_EXPORT phone_status_t phone_call_answer_after_id(phone_t instance, const char *call_id, int *answer_after);
        int phone_call_answer_after_id(Pointer phone, String call_id, IntByReference answer_after);

        // PHONE_EXPORT const char* phone_last_error(void);
        String phone_last_error();

        // PHONE_EXPORT void phone_status_name(char *out, size_t buffer_size, int code);
        void phone_status_name(byte[] out, NativeLong buffer_size, int state);

        // PHONE_EXPORT void phone_state_name(char *state_name, size_t buffer_size, int state);
        void phone_call_state_name(byte[] state_name, NativeLong buffer_size, int state);

        // PHONE_EXPORT void phone_set_log_level(int level);
        void phone_set_log_level(int level);

        // PHONE_EXPORT void phone_set_log_function(phone_t instance, void (*fn)(int level, const char *message, long thread_id, const char *thread_name));
        void phone_set_log_function(Pointer instance, LogFunction logFunction);

        // PHONE_EXPORT phone_status_t phone_register_thread(phone_t instance, const char *name);
        int phone_register_thread(Pointer instance, String name);

        // PHONE_EXPORT int phone_is_thread_registered(phone_t instance);
        boolean phone_is_thread_registered(Pointer instance);

        // PHONE_EXPORT phone_status_t phone_play_call_waiting(phone_t instance);
        int phone_play_call_waiting(Pointer instance);
        // PHONE_EXPORT phone_status_t phone_stop_call_waiting(phone_t instance);
        int phone_stop_call_waiting(Pointer instance);

        // PHONE_EXPORT unsigned phone_get_call_count(phone_t instance);
        int phone_get_call_count(Pointer instance);

        // PHONE_EXPORT phone_status_t phone_handle_ip_change(void);
        int phone_handle_ip_change();


        class AudioDeviceInfo extends Structure {
            public int id;
            public Pointer driver;
            public Pointer name;
            public int inputCount;
            public int outputCount;

            @Override
            protected List<String> getFieldOrder() {
                return Arrays.asList("id", "driver", "name", "inputCount", "outputCount");
            }
        }
    }

    static final PhoneLibrary CPHONE;

    static {
        String osName = System.getProperty("os.name").toLowerCase();
        if (osName.contains("mac")) {
            CPHONE = Native.load("libphone.0.dylib", PhoneLibrary.class);
        } else if (osName.contains("linux")) {
            CPHONE = Native.load("libphone.so.0", PhoneLibrary.class);
        } else {
            throw new RuntimeException("Unsupported OS: " + osName);
        }
    }

    static String describeStatus(int code) {
        byte[] buffer = new byte[32];
        CPHONE.phone_status_name(buffer, new NativeLong(buffer.length), code);
        return Native.toString(buffer);
    }

    static String getState(int state) {
        byte[] buffer = new byte[32];
        CPHONE.phone_call_state_name(buffer, new NativeLong(buffer.length), state);
        return Native.toString(buffer);
    }

    static String lastError() {
        return CPHONE.phone_last_error();
    }

    static void setLogLevel(int logLevel) {
        CPHONE.phone_set_log_level(logLevel);
    }

    final private Pointer phone;

    public Phone(String userAgent, String[] nameservers, String[] stunservers) throws PhoneException {
        this.phone = CPHONE.phone_create(userAgent, nameservers, new NativeLong(nameservers.length), stunservers, new NativeLong(stunservers.length));
        if(this.phone == null) {
            throw new PhoneException(lastError());
        }
    }

    public Phone(String userAgent, String[] stunservers) throws PhoneException {
        this.phone = CPHONE.phone_create_with_system_nameserver(userAgent, stunservers, new NativeLong(stunservers.length));
        if(this.phone == null) {
            throw new PhoneException(lastError());
        }
    }

    void destroy() {
        CPHONE.phone_destroy(phone);
    }

    void configureOpus(int channelCount, int complexity, int sampleRate) throws PhoneException {
        if (CPHONE.phone_configure_opus(phone, channelCount, complexity, sampleRate) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void connect(String server, String username, String password) throws PhoneException {
        if (CPHONE.phone_connect(phone, server, username, password) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void call(String number) throws PhoneException {
        if (CPHONE.phone_make_call(phone, number) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void setLogFunction(PhoneLibrary.LogFunction logFunction) {
        CPHONE.phone_set_log_function(phone, logFunction);
    }

    void registerOnRegistrationStateCallback(PhoneLibrary.RegistrationStateCallback cb) {
        CPHONE.phone_register_on_registration_state_callback(phone, cb, null);
    }

    void registerOnIncomingCallIndexCallback(PhoneLibrary.IncomingCallIndexCallback cb) {
        CPHONE.phone_register_on_incoming_call_index_callback(phone, cb, null);
    }

    void registerOnIncomingCallIdCallback(PhoneLibrary.IncomingCallIdCallback cb) {
        CPHONE.phone_register_on_incoming_call_id_callback(phone, cb, null);
    }

    void registerOnCallStateIndexCallback(PhoneLibrary.CallStateIndexCallback cb) {
        CPHONE.phone_register_on_call_state_index_callback(phone, cb, null);
    }

    void registerOnCallStateIdCallback(PhoneLibrary.CallStateIdCallback cb) {
        CPHONE.phone_register_on_call_state_id_callback(phone, cb, null);
    }

    void hangup(int callIndex) throws PhoneException {
        if (CPHONE.phone_hangup_call_index(phone, callIndex) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void hangup(String callId) throws PhoneException {
        if (CPHONE.phone_hangup_call_id(phone, callId) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void dtmf(int callIndex, String digits) throws PhoneException {
        if (CPHONE.phone_play_dtmf_call_index(phone, callIndex, digits) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void dtmf(String callId, String digits) throws PhoneException {
        if (CPHONE.phone_play_dtmf_call_id(phone, callId, digits) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void hangupCalls() {
        CPHONE.phone_hangup_calls(phone);
    }

    String getCallId(int callIndex) throws PhoneException {
        byte[] buffer = new byte[128];
        if (CPHONE.phone_get_call_id(phone, callIndex, buffer, new NativeLong(buffer.length)) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
        return Native.toString(buffer);
    }
    int getCallIndex(String callId) throws PhoneException {
        IntByReference iRef = new IntByReference();
        if (CPHONE.phone_get_call_index(phone, callId, iRef) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
        return iRef.getValue();
    }

    void answer(int callIndex) throws PhoneException {
        if (CPHONE.phone_answer_call_index(phone, callIndex) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void answer(String callId) throws PhoneException {
        if (CPHONE.phone_answer_call_id(phone, callId) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void startRinging(int callIndex) throws PhoneException {
        if (CPHONE.phone_start_ringing_call_index(phone, callIndex) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void startRinging(String callId) throws PhoneException {
        if (CPHONE.phone_start_ringing_call_id(phone, callId) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    int answerAfter(int callIndex) throws PhoneException {
        IntByReference iRef = new IntByReference();
        if (CPHONE.phone_call_answer_after_index(phone, callIndex, iRef) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
        return iRef.getValue();
    }

    int answerAfter(String callId) throws PhoneException {
        IntByReference iRef = new IntByReference();
        if (CPHONE.phone_call_answer_after_id(phone, callId, iRef) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
        return iRef.getValue();
    }

    static Phone.AudioDeviceInfo[] getAudioDevices(DeviceFilter filter) throws PhoneException {
        CPHONE.phone_refresh_audio_devices();

        long _count = CPHONE.phone_get_audio_devices_count().longValue();
        var audioDevicesCount = new LongByReference(_count);
        long driverNameLength = CPHONE.phone_get_audio_device_driver_name_length().longValue() + 1;
        long deviceNameLength = CPHONE.phone_get_audio_device_info_name_length().longValue() + 1;

        PhoneLibrary.AudioDeviceInfo info = new PhoneLibrary.AudioDeviceInfo();
        PhoneLibrary.AudioDeviceInfo[] infoArray = (PhoneLibrary.AudioDeviceInfo[])info.toArray((int) _count);

        Memory driverNames = new Memory(_count * driverNameLength);
        Memory deviceNames = new Memory(_count * deviceNameLength);

        for (int i = 0; i < _count; i++) {
            infoArray[i].driver = driverNames.share(i * driverNameLength);
            infoArray[i].name = deviceNames.share(i * deviceNameLength);
        }

        System.out.println("filter: " + filter);
        if (CPHONE.phone_get_audio_devices(infoArray, audioDevicesCount, new NativeLong(driverNameLength), new NativeLong(deviceNameLength), filter.getIntValue()) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }

        return Arrays.stream(Arrays.copyOf(infoArray, (int)audioDevicesCount.getValue())).map((audioDeviceInfo) -> new AudioDeviceInfo(
                audioDeviceInfo.id,
                Native.toString(audioDeviceInfo.driver.getByteArray(0, (int)driverNameLength)),
                Native.toString(audioDeviceInfo.name.getByteArray(0, (int)deviceNameLength)),
                audioDeviceInfo.inputCount,
                audioDeviceInfo.outputCount
        )).toArray(Phone.AudioDeviceInfo[]::new);
    }

    static void setAudioDevices(int captureDevice, int playbackDevice) throws PhoneException {
        if (CPHONE.phone_set_audio_devices(captureDevice, playbackDevice) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void registerThread(String name) {
        CPHONE.phone_register_thread(phone, name);
    }

    boolean isThreadRegistered() {
        return CPHONE.phone_is_thread_registered(phone);
    }

    void playCallWaiting() throws PhoneException {
        if (CPHONE.phone_play_call_waiting(phone) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    void stopCallWaiting() throws PhoneException {
        if (CPHONE.phone_stop_call_waiting(phone) != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }

    int callCount() {
        return CPHONE.phone_get_call_count(phone);
    }

    void handleIpChange() throws PhoneException {
        if (CPHONE.phone_handle_ip_change() != PHONE_STATUS_SUCCESS) {
            throw new PhoneException(lastError());
        }
    }
}

//
//  Model.swift
//  MultiPhone
//
//  Created by Oliver Epper on 24.04.23.
//

import Phone
import SwiftUI
import CoreAudio
import Foundation

#if os(macOS)
var deviceChangedAddress = AudioObjectPropertyAddress(
    mSelector: kAudioHardwarePropertyDevices,
    mScope: kAudioObjectPropertyScopeGlobal,
    mElement: kAudioObjectPropertyElementMain
)

func audioDeviceChanged(objectID: AudioObjectID, numberAddresses: UInt32, addresses: UnsafePointer<AudioObjectPropertyAddress>, context: UnsafeMutableRawPointer?) -> OSStatus {
    guard let context else { fatalError() }
    print("@@@@@" + #function)

    let my = Unmanaged<AppModel>.fromOpaque(context).takeUnretainedValue()
    my.withPhone { phone in
        try phone.call("+491804100100")
        print("@@@@@ -> isThreadRegistered: \(phone.isThreadRegistered)")
    }
    return noErr
}
#endif

final class AppModel: ObservableObject {
    @AppStorage("server") var server: String = ""
    @AppStorage("user") var username: String = ""

    @Published var errorMessage: String? = nil
    @Published var isConnected = false

    // https://christiantietze.de/posts/2023/01/entity-vs-value-object-and-identifiable-vs-equatable/
    @Published var calls: [Call.ID: Call] = [:]
    private var phone: Phone?
    private var lock = os_unfair_lock()

    private func setup() {
        do {
            phone = try Phone(userAgent: "MultiPhone ☎️")
            try phone?.configureOpus()
            phone?.registerOnRegistrationStateCallback({ isRegistered, registrationState in
                DispatchQueue.main.async {
                    self.isConnected = isRegistered
                    if !self.isConnected {
                        self.errorMessage = "Registration failed with: \(Phone.status(registrationState))"
                    }
                }
            })
        } catch Phone.Error.initialization {
            errorMessage = "Could not initialize Phone"
        } catch let Phone.Error.upstream(message) {
            errorMessage = message
        } catch { fatalError() }

        phone?.onIncomingCallCallback = { call in
            DispatchQueue.main.async {
                self.calls.updateValue(call, forKey: call.id)
            }
        }

        phone?.onCallStateCallback = { call in
            DispatchQueue.main.async {
                self.calls.removeValue(forKey: call.id)
                if !call.isDisconnected {
                    self.calls.updateValue(call, forKey: call.id)
                }
            }
        }

    }

    init() {
        os_unfair_lock_lock(&lock)
        defer {
            os_unfair_lock_unlock(&lock)
        }
        setup()
#if os(macOS)
        let status = AudioObjectAddPropertyListener(AudioObjectID(kAudioObjectSystemObject), &deviceChangedAddress, audioDeviceChanged, Unmanaged.passUnretained(self).toOpaque())
        guard status == noErr else {
            fatalError()

        }
#endif
    }

    func disconnect() {
        os_unfair_lock_lock(&lock)
        defer {
            os_unfair_lock_unlock(&lock)
        }
        self.phone = nil
        setup()
    }

    func withPhone(_ block: (Phone) throws -> Void) {
        os_unfair_lock_lock(&lock)
        defer {
            os_unfair_lock_unlock(&lock)
        }
        do {
            guard let phone else { fatalError() }
            if !phone.isThreadRegistered {
                try phone.registerThread()
            }
            try block(phone)
            self.setError(nil)
        } catch let Phone.Error.upstream(message) {
            self.setError(message)
        } catch { fatalError() }
    }

    func setError(_ message: String?) {
        DispatchQueue.main.async {
            self.errorMessage = message
        }
    }
}

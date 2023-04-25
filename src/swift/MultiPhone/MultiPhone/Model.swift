//
//  Model.swift
//  MultiPhone
//
//  Created by Oliver Epper on 24.04.23.
//

import Phone
import SwiftUI
import AVFoundation

final class Model: ObservableObject {
    @AppStorage("server") var server: String = ""
    @AppStorage("user") var username: String = ""

    @Published var errorMessage: String? = nil
    @Published var isConnected = false

#if os(iOS)
    @Published var isSpeakerEnabled = false
#endif

    private var phone: Phone?

    init() {
        do {
            phone = try Phone(userAgent: "MultiPhone ☎️")
            try phone?.configureOpus()
            phone?.registerOnRegistrationStateCallback({ isRegistered, registrationState in
                DispatchQueue.main.async {
                    self.isConnected = isRegistered
                    if !self.isConnected {
                        self.errorMessage = "Registration failed with: \(Phone.status(registrationState))"
                    } else {
                        self.errorMessage = nil
                    }
                }
            })
        } catch Phone.Error.initialization {
            self.errorMessage = "Could not initialize Phone"
        } catch let Phone.Error.upstream(message) {
            self.errorMessage = message
        } catch { fatalError() }
    }

    func withPhone(_ block: @escaping (Phone) throws -> Void) {
        do {
            guard let phone else { fatalError() }
            try block(phone)
        } catch let Phone.Error.upstream(message) {
            self.errorMessage = message
        } catch { fatalError() }
    }

#if os(iOS)
    func toggleSpeaker() {
        let audioSession = AVAudioSession.sharedInstance()
        do {
            if isSpeakerEnabled {
                try audioSession.setCategory(.playAndRecord, mode: .voiceChat, options: [])
            } else {
                try audioSession.setCategory(.playAndRecord, mode: .voiceChat, options: .defaultToSpeaker)
            }
            try audioSession.setActive(true, options: .notifyOthersOnDeactivation)
            isSpeakerEnabled.toggle()
        } catch {
            errorMessage = error.localizedDescription
        }
    }
#endif
}

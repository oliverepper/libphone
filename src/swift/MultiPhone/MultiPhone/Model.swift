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
    @Published var isConnected = false
    @Published var errorMessage: String? = nil

    private var phone: Phone?

    init() {
        do {
            phone = try Phone(userAgent: "MultiPhone ☎️")
            try phone?.configureOpus()
            phone?.registerOnRegistrationStateCallback({ isRegistered, registrationState in
                DispatchQueue.main.async {
                    self.isConnected = isRegistered
                }
            })
        } catch Phone.Error.initialization {
            self.errorMessage = "Could not initialize Phone"
        } catch let Phone.Error.upstream(message) {
            self.errorMessage = message
        } catch { fatalError() }
    }

    func connect(server: String, username: String, password: String) {
        do {
            try phone?.connect(server: server, username: username, password: password)
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

    func enableSpeaker() {
        let audioSession = AVAudioSession.sharedInstance()
        do {
            try audioSession.setCategory(.playAndRecord, mode: .voiceChat, options: .defaultToSpeaker)
            try audioSession.setActive(true, options: .notifyOthersOnDeactivation)
        } catch {
            errorMessage = error.localizedDescription
        }
    }
}

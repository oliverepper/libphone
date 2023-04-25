//
//  Model.swift
//  MultiPhone
//
//  Created by Oliver Epper on 24.04.23.
//

import Phone
import SwiftUI


final class AppModel: ObservableObject {
    @AppStorage("server") var server: String = ""
    @AppStorage("user") var username: String = ""

    @Published var errorMessage: String? = nil
    @Published var isConnected = false

    @Published var calls = Set<Call>()

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
                self.calls.insert(call)
            }
        }

        phone?.onCallStateCallback = { call in
            DispatchQueue.main.async {
                self.calls.remove(call)
                if !call.isDisconnected {
                    self.calls.insert(call)
                }
            }
        }
    }

    func withPhone(_ block: @escaping (Phone) throws -> Void) {
        do {
            guard let phone else { fatalError() }
            try block(phone)
            self.errorMessage = nil
        } catch let Phone.Error.upstream(message) {
            self.errorMessage = message
        } catch { fatalError() }
    }

    func setError(_ message: String) {
        DispatchQueue.main.async {
            self.errorMessage = message
        }
    }
}

//
//  MultiPhoneApp.swift
//  MultiPhone
//
//  Created by Oliver Epper on 24.04.23.
//

import SwiftUI

final class LastError: ObservableObject {
    @Published var message: String? = nil
}

//struct LastErrorKey: EnvironmentKey {
//    static let defaultValue = LastError()
//}
//
//extension EnvironmentValues {
//    var lastError: LastError {
//        get { self[LastErrorKey.self] }
//        set { self[LastErrorKey.self] = newValue }
//    }
//}

@main
struct MultiPhoneApp: App {
    @StateObject private var lastError = LastError()

    var body: some Scene {
        WindowGroup {
            ContentView().environmentObject(lastError)
        }
    }
}

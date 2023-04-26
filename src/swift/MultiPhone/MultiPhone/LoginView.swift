//
//  LoginView.swift
//  MultiPhone
//
//  Created by Oliver Epper on 24.04.23.
//

import SwiftUI
import Phone

struct LoginView: View {
    @ObservedObject var appModel: AppModel
    @State private var password = ""

    private static let passwordKeyBase = ProcessInfo.processInfo.processName

    public static func passwordKey(server: String, username: String) -> String {
        return [passwordKeyBase, server, username].joined(separator: "/")
    }

    var body: some View {
        VStack {
            TextField("Server", text: $appModel.server)
            TextField("Username", text: $appModel.username)
            SecureField("Password", text: $password)

            Button("Login") {
                appModel.withPhone { phone in
                    try phone.connect(server: appModel.server, username: appModel.username, password: password)
                }
            }
        }
        .padding()
        .onAppear {
            appModel.withPhone { phone in
                try phone.connect(server: appModel.server, username: appModel.username, password: password)
            }
        }
    }
}

struct LoginView_Previews: PreviewProvider {
    static var previews: some View {
        LoginView(appModel: AppModel())
    }
}

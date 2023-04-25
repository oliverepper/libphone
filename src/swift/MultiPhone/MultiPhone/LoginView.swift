//
//  LoginView.swift
//  MultiPhone
//
//  Created by Oliver Epper on 24.04.23.
//

import SwiftUI
import Phone

struct LoginView: View {
    @ObservedObject var model: AppModel
    @State private var password = ""

    private static let passwordKeyBase = ProcessInfo.processInfo.processName

    public static func passwordKey(server: String, username: String) -> String {
        return [passwordKeyBase, server, username].joined(separator: "/")
    }

    var body: some View {
        VStack {
            if let errorMessage = model.errorMessage {
                Text(verbatim: errorMessage).foregroundColor(.red)
            }
            TextField("Server", text: $model.server)
            TextField("Username", text: $model.username)
            SecureField("Password", text: $password)

            Button("Login") {
                model.withPhone { phone in
                    try phone.connect(server: model.server, username: model.username, password: password)
                }
            }
        }.padding()
    }
}

struct LoginView_Previews: PreviewProvider {
    static var previews: some View {
        LoginView(model: AppModel())
    }
}

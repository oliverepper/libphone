//
//  LoginView.swift
//  MultiPhone
//
//  Created by Oliver Epper on 24.04.23.
//

import SwiftUI
import Phone

struct LoginView: View {
    @State var model: Model
    @State private var password = ""

    private static let passwordKeyBase = ProcessInfo.processInfo.processName

    var body: some View {
        VStack {
            TextField("Server", text: $model.server)
            TextField("Username", text: $model.username)
            SecureField("Password", text: $password)

            Button("Login") {
                model.connect(server: model.server, username: model.username, password: password)
            }
        }.padding()
    }

    public static func passwordKey(server: String, username: String) -> String {
        return [passwordKeyBase, server, username].joined(separator: "/")
    }
}

struct LoginView_Previews: PreviewProvider {
    static var previews: some View {
        LoginView(model: Model())
    }
}

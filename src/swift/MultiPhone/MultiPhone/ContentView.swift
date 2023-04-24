//
//  ContentView.swift
//  MultiPhone
//
//  Created by Oliver Epper on 24.04.23.
//

import SwiftUI
import Phone


struct ContentView: View {
    @StateObject var model = Model()

    var body: some View {
        VStack {
            if let errorMessage = model.errorMessage {
                Text(verbatim: errorMessage).foregroundColor(.red)
            }
            if !model.isConnected {
                LoginView(model: self.model)
            } else {
                Group {
                    Text(verbatim: "\(model.username) connected to \(model.server)")
                    Button("Call Zeitansage") {
                        model.withPhone { phone in
                            try phone.call("+491804100100")
                        }
                    }
                    Button("Enable Speaker") {
                        model.enableSpeaker()
                    }
                }.padding()
            }
        }
        .padding()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}

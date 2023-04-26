//
//  ContentView.swift
//  MultiPhone
//
//  Created by Oliver Epper on 24.04.23.
//

import SwiftUI
import Phone


struct ContentView: View {
    @StateObject var appModel = AppModel()
    @Environment(\.scenePhase) var scenePhase

    var body: some View {
        VStack {
            Spacer()
            if let errorMessage = appModel.errorMessage {
                Text(verbatim: errorMessage).foregroundColor(.red)
            }
            if !appModel.isConnected {
                LoginView(appModel: self.appModel)
            } else {
                MainView(appModel: self.appModel)
            }
            Spacer()
            BuildInfo(leftText: Phone.version + "\n" + Phone.gitHash + "\n" + Phone.gitDescription).padding(.horizontal)
        }
        .padding()
        .onChange(of: scenePhase) { newValue in
            switch newValue {
            case .active:
                print("@@@@@ ACTIVE")
            case .inactive:
                print("@@@@@ INACTIVE")
                appModel.disconnect()
            case .background:
                print("@@@@@ BACKGROUND")
            @unknown default:
                fatalError()
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}

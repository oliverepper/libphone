//
//  MainView.swift
//  MultiPhone
//
//  Created by Oliver Epper on 25.04.23.
//

import Phone
import SwiftUI


struct MainView: View {
    @ObservedObject var appModel: AppModel

    var body: some View {
        VStack {
            Text(verbatim: "\(appModel.username) connected to \(appModel.server)")
            Group {
                Button("Call Zeitansage") {
                    appModel.withPhone { phone in
                        try phone.call("+491804100100")
                    }
                }
#if os(iOS)
                EnableSpeakerView()
#endif
                Button("disconnect") {
                    appModel.disconnect()
                }
            }.padding()
            ForEach(Array(appModel.calls.values)) { call in
                HStack {
                    Text(verbatim: call.description)
                    if [Call.State.incoming, Call.State.early].contains(call.state) {
                        Button("answer") {
                            do {
                                try call.answer()
                                appModel.errorMessage = nil
                            } catch let Phone.Error.upstream(message) {
                                appModel.setError(message)
                            } catch { fatalError() }
                        }
                    }
                    Button("hangup") {
                        do {
                            try call.hangup()
                            appModel.errorMessage = nil
                        } catch let Phone.Error.upstream(message) {
                            appModel.setError(message)
                        } catch { fatalError() }
                    }
                }
            }
        }
    }
}

struct MainView_Previews: PreviewProvider {
    static var previews: some View {
        MainView(appModel: AppModel())
    }
}

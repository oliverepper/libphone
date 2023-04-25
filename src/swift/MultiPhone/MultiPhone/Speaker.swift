//
//  SpeakerModel.swift
//  MultiPhone
//
//  Created by Oliver Epper on 25.04.23.
//

import SwiftUI
import AVFoundation

#if os(iOS)
final class SpeakerModel: ObservableObject {
    @Published var isSpeakerEnabled = false
    @EnvironmentObject var lastError: LastError

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
            lastError.message = error.localizedDescription
        }
    }
}

struct EnableSpeakerView: View {
    @StateObject var model = SpeakerModel()

    var body: some View {
        Button(!model.isSpeakerEnabled ? "Enable Speaker" : "Disable Speaker") {
            model.toggleSpeaker()
        }
    }
}
#endif

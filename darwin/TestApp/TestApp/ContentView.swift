//
//  ContentView.swift
//  TestApp
//
//  Created by Oliver Epper on 19.07.24.
//

import SwiftUI

class Model: ObservableObject {
    @Published var version = "libphone \(phone_version_major()).\(phone_version_minor()).\(phone_version_patch())"
}


struct ContentView: View {
    @State private var model = Model()

    var body: some View {
        Text(verbatim: model.version).padding()
    }
}

#Preview {
    ContentView()
}

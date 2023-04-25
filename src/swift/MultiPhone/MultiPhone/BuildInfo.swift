//
//  BuildInfo.swift
//  MultiPhone
//
//  Created by Oliver Epper on 25.04.23.
//

import SwiftUI

public struct BuildInfo: View {
    static let dateFormatter: DateFormatter = {
        let formatter = DateFormatter()
        formatter.dateStyle = .full
        return formatter
    }()

    let version = Bundle.main.infoDictionary?["CFBundleShortVersionString"] as? String ?? "Unknown ShortVersion"
    let build = Bundle.main.infoDictionary?["CFBundleVersion"] as? String ?? "Unknown Version"
    let leftText: String?

    public init(leftText: String? = nil) {
        self.leftText = leftText
    }

    public var body: some View {
        HStack(alignment: .top) {
            if let leftText { Text(verbatim: leftText) }
            Spacer()
            VStack(alignment: .trailing) {
                Text("Version: \(version) Build: \(build)")
                if let date = buildDate {
                    Text(Self.dateFormatter.string(from: date))
                }
            }
        }.font(.footnote)
    }

    private var buildDate: Date? {
        guard let infoPath = Bundle.main.path(forResource: "Info", ofType: "plist"),
        let infoAttr = try? FileManager.default.attributesOfItem(atPath: infoPath),
        let infoDate = infoAttr[.modificationDate] as? Date else {
            return nil
        }
        return infoDate
    }
}

struct BuildInfo_Previews: PreviewProvider {
    static var previews: some View {
        BuildInfo()
    }
}

//
//  CallModel.swift
//  MultiPhone
//
//  Created by Oliver Epper on 25.04.23.
//

import cphone

public final class Call: Identifiable {
    private let phone: phone_t
    public let id: String

    init(phone: phone_t, id: String) {
        self.phone = phone
        self.id = id
    }

    public func answer() {
        phone_answer_call_id(phone, id)
    }

    public func hangup() {
        phone_hangup_call_id(phone, id)
    }
}

extension Call: CustomStringConvertible {
    public var description: String {
        "Call <\(self.id)>"
    }
}

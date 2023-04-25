//
//  CallModel.swift
//  MultiPhone
//
//  Created by Oliver Epper on 25.04.23.
//

import cphone

public struct Call: Identifiable, Hashable {

    public enum State: Int32 {
        case null           = 0
        case calling        = 1
        case incoming       = 2
        case early          = 3
        case connecting     = 4
        case confirmed      = 5
        case disconnected   = 6

        public static func ==(lhs: State, rhs: Int32) -> Bool {
            return lhs.rawValue == rhs
        }
    }

    public let id: String
    private let state: Int32

    private let phone: phone_t

    init(phone: phone_t, id: String, state: Int32 = 0) {
        self.phone = phone
        self.id = id
        self.state = state
    }

    public func answer() throws {
        if phone_answer_call_id(phone, id) != PHONE_STATUS_SUCCESS {
            throw Phone.Error.upstream(.init(cString: phone_last_error()))
        }
    }

    public func hangup() throws {
        if phone_hangup_call_id(phone, id) != PHONE_STATUS_SUCCESS {
            throw Phone.Error.upstream(.init(cString: phone_last_error()))
        }
    }

    public static func ==(lhs: Call, rhs: Call) -> Bool {
        return lhs.id == rhs.id
    }

    public func hash(into hasher: inout Hasher) {
        hasher.combine(self.id)
    }
}

extension Call: CustomStringConvertible {
    public var description: String {
        "Call <\(self.id)> - \(self.status)"
    }
}

extension Call {
    public var status: String {
        let buffer = UnsafeMutablePointer<CChar>.allocate(capacity: 128)
        phone_call_state_name(buffer, 128, self.state)
        defer {
            buffer.deallocate()
        }
        return .init(cString: buffer)
    }

    public var isDisconnected: Bool {
        return self.state == State.disconnected.rawValue
    }
}

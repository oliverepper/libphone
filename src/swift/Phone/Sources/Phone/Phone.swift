import cphone

public final class Phone {
    public enum Error: Swift.Error {
        case initialization
        case upstream(String)
    }

    class OnRegistrationStateFunctor {
        let callback: (Bool, Int32) -> Void

        init(_ callback: @escaping (Bool, Int32) -> Void) {
            self.callback = callback
        }

        func callAsFunction(isRegistered: Bool, registrationState: Int32) {
            callback(isRegistered, registrationState)
        }
    }

    private var phone: phone_t!

    public init(userAgent: String, nameserver: [String] = [], stunserver: [String] = []) throws {
        let userAgentCString = strdup(userAgent)
        let nameserverCStrings = nameserver.map { UnsafePointer($0.withCString(strdup)) }
        let stunserverCStrings = stunserver.map { UnsafePointer($0.withCString(strdup)) }
        defer {
            free(userAgentCString)
            for str in nameserverCStrings { free(UnsafeMutableRawPointer(mutating: str)) }
            for str in stunserverCStrings { free(UnsafeMutableRawPointer(mutating: str)) }
        }

        self.phone = phone_create(userAgentCString, nameserverCStrings, nameserver.count, stunserverCStrings, stunserver.count)
        if self.phone == nil { throw Error.initialization }
    }

    deinit {
        phone_destroy(self.phone)
    }

    public func configureOpus(channelCount: Int32 = 1, complexity: Int32 = 8, sampleRate: Int32 = 16_000) throws {
        if phone_configure_opus(self.phone, channelCount, complexity, sampleRate) != PHONE_STATUS_SUCCESS {
            throw Error.upstream(.init(cString: phone_last_error()))
        }
     }

    public func connect(server: String, username: String, password: String) throws {
        if phone_connect(self.phone, server, username, password) != PHONE_STATUS_SUCCESS {
            throw Error.upstream(.init(cString: phone_last_error()))
        }
    }

    public func registerOnRegistrationStateCallback(_ callback: @escaping (Bool, Int32) -> Void) {
        phone_register_on_registration_state_callback(self.phone, { is_registered, registration_state, ctx in
            guard let ctx else { return }
            let onRegistrationStateFunctor = Unmanaged<OnRegistrationStateFunctor>.fromOpaque(ctx).takeUnretainedValue()
            onRegistrationStateFunctor(isRegistered: is_registered != 0, registrationState: registration_state)
        }, Unmanaged.passRetained(OnRegistrationStateFunctor(callback)).toOpaque())
    }

    public func call(_ uri: String) throws {
        if phone_make_call(self.phone, uri) != PHONE_STATUS_SUCCESS {
            throw Error.upstream(.init(cString: phone_last_error()))
        }
    }
}

extension Phone {
    public static func status(_ code: Int32) -> String {
        let buffer = UnsafeMutablePointer<CChar>.allocate(capacity: 128)
        phone_status_name(buffer, 128, code)
        defer {
            buffer.deallocate()
        }
        return .init(cString: buffer)
    }
}

extension Phone {
    public static var version: String {
        let versionString = "libphone-" +
        [
            phone_version_major(),
            phone_version_minor(),
            phone_version_patch(),
        ].map(String.init).joined(separator: ".")

        return phone_version_tweak() == 0 ? versionString : versionString + "-\(phone_version_tweak())"
    }

    public static var gitHash: String {
        let buffer = UnsafeMutablePointer<CChar>.allocate(capacity: 128)
        phone_git_hash(buffer, 128)
        defer {
            buffer.deallocate()
        }
        return .init(cString: buffer)
    }

    public static var gitDescription: String {
        let buffer = UnsafeMutablePointer<CChar>.allocate(capacity: 128)
        phone_git_description(buffer, 128)
        defer {
            buffer.deallocate()
        }
        return .init(cString: buffer)
    }
}

import XCTest
@testable import Phone

final class PhoneTests: XCTestCase {
    func testCreatePhone() throws {
        let phone = try? Phone(userAgent: "Test Phone ☎️")
        XCTAssertNotNil(phone)
    }

    func testConfigureOpusWithError() throws {
        let phone = try Phone(userAgent: "Test Phone ☎️")
        XCTAssertThrowsError(try phone.configureOpus(sampleRate: 16_00))
    }

    func testConnectWithWrongCredentials() throws {
        let phone = try Phone(userAgent: "Test Phone ☎️")
        try phone.connect(server: "v7oliep.starface-cloud.com", username: "stdsip", password: "bad_password")
        // FIXME: add callback and check for the FORBIDDEN code
    }
}

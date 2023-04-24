// swift-tools-version: 5.8

import PackageDescription

let package = Package(
    name: "Phone",
    products: [
        .library(name: "Phone", targets: ["Phone"]),
    ],
    targets: [
        .binaryTarget(name: "libphone",
                      url: "https://github.com/oliverepper/libphone/releases/download/0.7.0/libphone-0.7.0.xcframework.zip",
                      checksum: "a06e85587d53e728ff5692f169cd182d7c17560c22beb16d1f43fa7793c256a0"),
        .systemLibrary(name: "cphone"),
        .target(name: "Phone",
                dependencies: ["cphone", "libphone"]),
        .testTarget(name: "PhoneTests", dependencies: ["Phone"]),
    ]
)

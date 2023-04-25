// swift-tools-version: 5.8

import PackageDescription

let package = Package(
    name: "Phone",
    products: [
        .library(name: "Phone", targets: ["Phone"]),
    ],
    targets: [
        .binaryTarget(name: "libphone",
                      url: "https://github.com/oliverepper/libphone/releases/download/0.7.0/libphone-0.7.0-debug.xcframework.zip",
                      checksum: "57aa4e13da0c05156a61d0d756a28049e9e4d5e9c787267bc4bb7282e51124d9"),
        .systemLibrary(name: "cphone"),
        .target(name: "Phone",
                dependencies: ["cphone", "libphone"]),
        .testTarget(name: "PhoneTests", dependencies: ["Phone"]),
    ]
)

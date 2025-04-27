// swift-tools-version:4.2

import PackageDescription

let package = Package(
    name: "CChipmunk2D",
    products: [
        .library(name: "CChipmunk2D", type: .static, targets: ["CChipmunk2D"])
    ],
    targets: [
        .target(name: "CChipmunk2D", path: ".", sources: ["src"], publicHeadersPath: "include")
    ]
)
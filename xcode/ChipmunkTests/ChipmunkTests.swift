//
//  ChipmunkTests.swift
//  ChipmunkTests
//
//  Created by Alsey Coleman Miller on 10/6/16.
//
//

import XCTest
@testable import Chipmunk

final class ChipmunkTests: XCTestCase {
    
    func testVector() {
        
        let vector = Chipmunk.Vector(x: 10, y: 10)
        
        XCTAssert(__cpvzero.x == 0 && __cpvzero.y == 0, "cpvzero should be imported as private")
        
    }
}

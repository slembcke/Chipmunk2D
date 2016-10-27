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
        
        XCTAssert(vector.isEqual(Chipmunk.Vector.zero) == false)
        
        XCTAssert(vector.add(Vector(x:5, y:5)) == Vector(x: 15, y: 15))
        
        XCTAssert(vector.negate == Vector(x: -10, y: -10))
    }
}

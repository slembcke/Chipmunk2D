//
//  ChipmunkTests.swift
//  ChipmunkTests
//
//  Created by Alsey Coleman Miller on 10/6/16.
//
//

import XCTest
import SwiftShims
@testable import Chipmunk

final class ChipmunkTests: XCTestCase {
    
    func testVector() {
        
        let vector = Chipmunk.Vector(x: 10, y: 10)
        
        XCTAssert(vector.isEqual(Chipmunk.Vector.zero) == false)
        
        XCTAssert(vector.add(Vector(x:5, y:5)) == Vector(x: 15, y: 15))
        
        XCTAssert(vector.negate == Vector(x: -10, y: -10))
    }
    
    func testAffineTransform() {
        
        var affineTransform = Chipmunk.AffineTransform.identity
        
        affineTransform = AffineTransform(translate: Vector.zero)
        
        affineTransform = AffineTransform.wrap(outer: affineTransform, inner: AffineTransform.identity)
    }
    
    func testBody() {
        
        let bodyType: Chipmunk.BodyType = .dynamic // properly import enum
        
        XCTAssert(bodyType.rawValue == 0)
        
        let body = Chipmunk.Body(mass: 1.0, moment: 1.0)
        
        let group = Body(mass: 123, moment: 123)
        
        defer {
            
            group.free()
            
            XCTAssert(_swift_stdlib_malloc_size(UnsafeRawPointer(group.pointer!)) == 0, "Memory was not freed: \(group)")
        }
        
        // need to assign to space first
        
        
        
        /*
        body.sleep()
        
        XCTAssert(body.isSleeping == true)
        
        body.sleep(with: group)
        
        XCTAssert(body.isSleeping == true)
        
        body.activate()
        
        XCTAssert(body.isSleeping == false)
        
        body.activateStatic(filter: Shape)
        
        XCTAssert(body.isSleeping == false)
        */
        body.bodyType = bodyType
        
        XCTAssert(body.bodyType == bodyType)
    }
}

//
//  cpShapeRef.h
//  Chipmunk7
//
//  Created by Alsey Coleman Miller on 10/7/16.
//
//

#import "chipmunk.h"

CP_ASSUME_NONNULL_BEGIN

/// Defines the shape of a rigid body.
///
/// - Note: Reference type.
typedef struct __attribute__((swift_name("Shape"))) cpShapeRef {
    cpShape * _Nullable pointer;
} cpShapeRef;



CP_ASSUME_NONNULL_END

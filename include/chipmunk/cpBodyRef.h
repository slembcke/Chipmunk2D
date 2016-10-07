//
//  cpBodyRef.h
//  Chipmunk7
//
//  Created by Alsey Coleman Miller on 10/7/16.
//
//

#import "cpBody.h"

CP_ASSUME_NONNULL_BEGIN

//typedef const struct CF_BRIDGED_TYPE(NSURL) cpBody * cpBodyRef;

/// Chipmunk's rigid body type.
///
/// - Note: Reference type.
typedef struct __attribute__((swift_name("Body"))) cpBodyRef {
    cpBody * pointer;
} cpBodyRef;

/*
__attribute__((swift_name("getter:Body.kinematic()")))
static inline cpBodyRef cpBodyRefCreateKinematic()
{
    cpBodyRef ref = { cpBodyNewKinematic() };
    return ref;
}
*/

/// Allocate and initialize a cpBodyRef.
__attribute__((swift_name("Body.init(mass:moment:)")))
static inline cpBodyRef cpBodyRefCreateWithMassAndMoment(cpFloat mass, cpFloat moment)
{
    cpBodyRef ref = { cpBodyNew(mass, moment) };
    return ref;
}

/// Force a body to fall asleep immediately along with other bodies in a group.
//CP_EXPORT void cpBodySleepWithGroup(cpBody *body, cpBody  * _Nullable group);

CP_ASSUME_NONNULL_END

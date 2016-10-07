//
//  cpBodyRef.h
//  Chipmunk7
//
//  Created by Alsey Coleman Miller on 10/7/16.
//
//

#import "chipmunk.h"
#import "cpShape.h"
#import "cpShapeRef.h"

CP_ASSUME_NONNULL_BEGIN

/// Chipmunk's rigid body type.
///
/// - Note: Reference type.
typedef struct __attribute__((swift_name("Body"))) cpBodyRef {
    cpBody * _Nullable pointer;
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

/// Free the pointer.
__attribute__((swift_name("Body.free(self:)")))
static inline void cpBodyRefFree(cpBodyRef body)
{
    cpBodyFree(body.pointer);
}

/// Wake up a sleeping or idle body.
__attribute__((swift_name("Body.activate(self:)")))
static inline void cpBodyRefActivate(cpBodyRef body)
{
    cpBodyActivate(body.pointer);
}

/// Wake up any sleeping or idle bodies touching a static body.
__attribute__((swift_name("Body.activateStatic(self:filter:)")))
static inline void cpBodyRefActivateStatic(cpBodyRef body, cpShapeRef filter)
{
    cpBodyActivateStatic(body.pointer, filter.pointer);
}

/// Force a body to fall asleep immediately.
__attribute__((swift_name("Body.sleep(self:)")))
static inline void cpBodyRefSleep(cpBodyRef body)
{
    cpBodySleep(body.pointer);
}

/// Force a body to fall asleep immediately along with other bodies in a group.
__attribute__((swift_name("Body.sleep(self:with:)")))
static inline void cpBodyRefSleepWithGroup(cpBodyRef body, cpBodyRef group)
{
    cpBodySleepWithGroup(body.pointer, group.pointer);
}

/// Returns `true` if the body is sleeping.
__attribute__((swift_name("getter:Body.isSleeping(self:)")))
static inline cpBool cpBodyRefIsSleeping(const cpBodyRef body)
{
    return cpBodyIsSleeping(body.pointer);
}

/// Get the type of the body.
__attribute__((swift_name("getter:Body.bodyType(self:)")))
static inline cpBodyType cpBodyRefGetType(cpBodyRef body)
{
    return cpBodyGetType(body.pointer);
}
/// Set the type of the body.
__attribute__((swift_name("setter:Body.bodyType(self:_:)")))
static inline void cpBodyRefSetType(cpBodyRef body, cpBodyType type)
{
    cpBodySetType(body.pointer, type);
}

/*
/// Get the space this body is added to.
__attribute__((swift_private))
static inline cpSpace* cpBodyGetSpace(const cpBodyRef body);

/// Get the mass of the body.
__attribute__((swift_private))
static inline cpFloat cpBodyGetMass(const cpBodyRef body);
/// Set the mass of the body.
__attribute__((swift_private))
static inline void cpBodyRefSetMass(cpBodyRef body, cpFloat m);

/// Get the moment of inertia of the body.
__attribute__((swift_private))
static inline cpFloat cpBodyGetMoment(const cpBodyRef body);
/// Set the moment of inertia of the body.
__attribute__((swift_private))
static inline void cpBodyRefSetMoment(cpBodyRef body, cpFloat i);

/// Set the position of a body.
__attribute__((swift_private))
static inline cpVect cpBodyGetPosition(const cpBodyRef body);
/// Set the position of the body.
__attribute__((swift_private))
static inline void cpBodyRefSetPosition(cpBodyRef body, cpVect pos);

/// Get the offset of the center of gravity in body local coordinates.
__attribute__((swift_private))
static inline cpVect cpBodyGetCenterOfGravity(const cpBodyRef body);
/// Set the offset of the center of gravity in body local coordinates.
__attribute__((swift_private))
static inline void cpBodyRefSetCenterOfGravity(cpBodyRef body, cpVect cog);

/// Get the velocity of the body.
__attribute__((swift_private))
static inline cpVect cpBodyGetVelocity(const cpBodyRef body);
/// Set the velocity of the body.
__attribute__((swift_private))
static inline void cpBodyRefSetVelocity(cpBodyRef body, cpVect velocity);

/// Get the force applied to the body for the next time step.
__attribute__((swift_private))
static inline cpVect cpBodyGetForce(const cpBodyRef body);
/// Set the force applied to the body for the next time step.
__attribute__((swift_private))
static inline void cpBodyRefSetForce(cpBodyRef body, cpVect force);

/// Get the angle of the body.
__attribute__((swift_private))
static inline cpFloat cpBodyGetAngle(const cpBodyRef body);
/// Set the angle of a body.
__attribute__((swift_private))
static inline void cpBodyRefSetAngle(cpBodyRef body, cpFloat a);

/// Get the angular velocity of the body.
__attribute__((swift_private))
static inline cpFloat cpBodyGetAngularVelocity(const cpBodyRef body);
/// Set the angular velocity of the body.
__attribute__((swift_private))
static inline void cpBodyRefSetAngularVelocity(cpBodyRef body, cpFloat angularVelocity);

/// Get the torque applied to the body for the next time step.
__attribute__((swift_private))
static inline cpFloat cpBodyGetTorque(const cpBodyRef body);
/// Set the torque applied to the body for the next time step.
__attribute__((swift_private))
static inline void cpBodyRefSetTorque(cpBodyRef body, cpFloat torque);

/// Get the rotation vector of the body. (The x basis vector of it's transform.)
__attribute__((swift_private))
static inline cpVect cpBodyGetRotation(const cpBodyRef body);

/// Get the user data pointer assigned to the body.
__attribute__((swift_private))
static inline cpDataPointer cpBodyGetUserData(const cpBodyRef body);
/// Set the user data pointer assigned to the body.
__attribute__((swift_private))
static inline void cpBodyRefSetUserData(cpBodyRef body, cpDataPointer userData);

/// Set the callback used to update a body's velocity.
__attribute__((swift_private))
static inline void cpBodyRefSetVelocityUpdateFunc(cpBodyRef body, cpBodyVelocityFunc velocityFunc);
/// Set the callback used to update a body's position.
/// NOTE: It's not generally recommended to override this unless you call the default position update function.
__attribute__((swift_private))
static inline void cpBodyRefSetPositionUpdateFunc(cpBodyRef body, cpBodyPositionFunc positionFunc);

/// Default velocity integration function..
__attribute__((swift_private))
static inline void cpBodyRefUpdateVelocity(cpBodyRef body, cpVect gravity, cpFloat damping, cpFloat dt);
/// Default position integration function.
__attribute__((swift_private))
static inline void cpBodyRefUpdatePosition(cpBodyRef body, cpFloat dt);

/// Convert body relative/local coordinates to absolute/world coordinates.
__attribute__((swift_private))
static inline cpVect cpBodyLocalToWorld(const cpBodyRef body, const cpVect point);
/// Convert body absolute/world coordinates to  relative/local coordinates.
__attribute__((swift_private))
static inline cpVect cpBodyWorldToLocal(const cpBodyRef body, const cpVect point);

/// Apply a force to a body. Both the force and point are expressed in world coordinates.
__attribute__((swift_private))
static inline void cpBodyRefApplyForceAtWorldPoint(cpBodyRef body, cpVect force, cpVect point);
/// Apply a force to a body. Both the force and point are expressed in body local coordinates.
__attribute__((swift_private))
static inline void cpBodyRefApplyForceAtLocalPoint(cpBodyRef body, cpVect force, cpVect point);

/// Apply an impulse to a body. Both the impulse and point are expressed in world coordinates.
__attribute__((swift_private))
static inline void cpBodyRefApplyImpulseAtWorldPoint(cpBodyRef body, cpVect impulse, cpVect point);
/// Apply an impulse to a body. Both the impulse and point are expressed in body local coordinates.
__attribute__((swift_private))
static inline void cpBodyRefApplyImpulseAtLocalPoint(cpBodyRef body, cpVect impulse, cpVect point);

/// Get the velocity on a body (in world units) at a point on the body in world coordinates.
__attribute__((swift_private))
static inline cpVect cpBodyGetVelocityAtWorldPoint(const cpBodyRef body, cpVect point);
/// Get the velocity on a body (in world units) at a point on the body in local coordinates.
__attribute__((swift_private))
static inline cpVect cpBodyGetVelocityAtLocalPoint(const cpBodyRef body, cpVect point);

/// Get the amount of kinetic energy contained by the body.
__attribute__((swift_private))
static inline cpFloat cpBodyKineticEnergy(const cpBodyRef body);

/// Body/shape iterator callback function type.
__attribute__((swift_private))
typedef void (*cpBodyShapeIteratorFunc)(cpBodyRef body, cpShape *shape, void *data);
/// Call @c func once for each shape attached to @c body and added to the space.
__attribute__((swift_private))
static inline void cpBodyRefEachShape(cpBodyRef body, cpBodyShapeIteratorFunc func, void *data);

/// Body/constraint iterator callback function type.
__attribute__((swift_private))
typedef void (*cpBodyConstraintIteratorFunc)(cpBodyRef body, cpConstraint *constraint, void *data);
/// Call @c func once for each constraint attached to @c body and added to the space.
__attribute__((swift_private))
static inline void cpBodyRefEachConstraint(cpBodyRef body, cpBodyConstraintIteratorFunc func, void *data);

/// Body/arbiter iterator callback function type.
__attribute__((swift_private))
typedef void (*cpBodyArbiterIteratorFunc)(cpBodyRef body, cpArbiter *arbiter, void *data);
/// Call @c func once for each arbiter that is currently active on the body.
__attribute__((swift_private))
static inline void cpBodyRefEachArbiter(cpBodyRef body, cpBodyArbiterIteratorFunc func, void *data);
 
 */

CP_ASSUME_NONNULL_END

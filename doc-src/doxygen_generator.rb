OUTPUT_NAME = "generated_docs"
exit if File.exists?(OUTPUT_NAME)

DOC_FILE = open(OUTPUT_NAME, "w")

FUNCS = {}
IO.readlines("|ruby extract_protos.rb").each{|line|
	func = eval(line)
	FUNCS[func[:name]] = func
}

SKIP = Object.new

GETTER_DOCS = {
	"cpArbiterGetBodies" => SKIP,
	"cpArbiterGetContactPointSet" => SKIP,
	"cpArbiterGetCount" => SKIP,
	"cpArbiterGetDepth" => SKIP,
	"cpArbiterGetElasticity" => "Get the elasticity for this cpArbiter",
	"cpArbiterGetFriction" => "Get the friction for this cpArbiter",
	"cpArbiterGetNormal" => SKIP,
	"cpArbiterGetPoint" => SKIP,
	"cpArbiterGetShapes" => SKIP,
	"cpArbiterGetSurfaceVelocity" => "Get the surface velocity used by the contact calculation for this cpArbiter",
	
	"cpBodyGetAngVel" => "Get the angular velocity of this cpBody",
	"cpBodyGetAngVelLimit" => "Get the angular velocity limit of this cpBody",
	"cpBodyGetAngle" => "Get the angle of this cpBody",
	"cpBodyGetForce" => "Get the force applied to this cpBody",
	"cpBodyGetMass" => "Get the mass of this cpBody",
	"cpBodyGetMoment" => "Get the moment of inertia of this cpBody",
	"cpBodyGetPos" => "Get the position of this cpBody",
	"cpBodyGetRot" => "Get the rotation vector of this cpBody",
	"cpBodyGetTorque" => "Get the torque applied to this cpBody",
	"cpBodyGetUserData" => "Get the userdata pointer for this cpBody",
	"cpBodyGetVel" => "Get the velocity of this cpBody",
	"cpBodyGetVelLimit" => "Get the velocity limit of this cpBody",
	
	"cpCircleShapeGetOffset" => "Get the offset of this cpCircleShape",
	"cpCircleShapeGetRadius" => "Get the radius of this cpCircleShape",
	
	"cpConstraintGetA" => "Get the first of the two bodies this cpConstraint is connected to.",
	"cpConstraintGetB" => "Get the second of the two bodies this cpConstraint is connected to.",
	"cpConstraintGetErrorBias" => "Get the percentage of constraint error that remains unfixed after each second.",
	"cpConstraintGetImpulse" => SKIP,
	"cpConstraintGetMaxBias" => "Get the maximum rate this cpConstraint can apply to correct itself at.",
	"cpConstraintGetMaxForce" => "Get the maximum force this cpConstraint can apply to correct itself.",
	"cpConstraintGetPostSolveFunc" => "Get the function callback that is called each step after the solver runs.",
	"cpConstraintGetPreSolveFunc" => "Get the function callback that is called each step before the solver runs.",
	"cpConstraintGetUserData" => "Get the user data pointer for this cpConstraint.",
	
	"cpDampedRotarySpringGetDamping" => "Get the damping of this cpDampedRotarySpring.",
	"cpDampedRotarySpringGetRestAngle" => "Get the restangle of this cpDampedRotarySpring.",
	"cpDampedRotarySpringGetSpringTorqueFunc" => "Get the springtorquefunc of this cpDampedRotarySpring.",
	"cpDampedRotarySpringGetStiffness" => "Get the stiffness of this cpDampedRotarySpring.",
	
	"cpDampedSpringGetAnchr1" => "Get the anchr1 of this cpDampedSpring.",
	"cpDampedSpringGetAnchr2" => "Get the anchr2 of this cpDampedSpring.",
	"cpDampedSpringGetDamping" => "Get the damping of this cpDampedSpring.",
	"cpDampedSpringGetRestLength" => "Get the rest length of this cpDampedSpring.",
	"cpDampedSpringGetSpringForceFunc" => "Get the spring force callback function of this cpDampedSpring.",
	"cpDampedSpringGetStiffness" => "Get the stiffness of this cpDampedSpring.",
	
	"cpGearJointGetPhase" => "Get the phase of this cpGearJoint.",
	"cpGearJointGetRatio" => "Get the ratio of this cpGearJoint.",
	
	"cpGrooveJointGetAnchr2" => "Get the anchr2 of this cpGrooveJoint.",
	"cpGrooveJointGetGrooveA" => "Get the groovea of this cpGrooveJoint.",
	"cpGrooveJointGetGrooveB" => "Get the grooveb of this cpGrooveJoint.",
	
	"cpPinJointGetAnchr1" => "Get the anchr1 of this cpPinJoint.",
	"cpPinJointGetAnchr2" => "Get the anchr2 of this cpPinJoint.",
	"cpPinJointGetDist" => "Get the dist between the anchor points of this cpPinJoint.",
	
	"cpPivotJointGetAnchr1" => "Get the anchr1 of this cpPivotJoint.",
	"cpPivotJointGetAnchr2" => "Get the anchr2 of this cpPivotJoint.",
	
	"cpPolyShapeGetNumVerts" => SKIP,
	"cpPolyShapeGetVert" => SKIP,
	
	"cpRatchetJointGetAngle" => "Get the angle of this cpRatchetJoint.",
	"cpRatchetJointGetPhase" => "Get the phase of this cpRatchetJoint.",
	"cpRatchetJointGetRatchet" => "Get the ratchet angular distance of this cpRatchetJoint.",
	
	"cpRotaryLimitJointGetMax" => "Get the max delta angle of this cpRotaryLimitJoint.",
	"cpRotaryLimitJointGetMin" => "Get the min delta angle of this cpRotaryLimitJoint.",
	
	"cpSegmentShapeGetA" => "Get the first endpoint of this cpSegmentShape.",
	"cpSegmentShapeGetB" => "Get the second endpoint of this cpSegmentShape.",
	"cpSegmentShapeGetNormal" => "Get the normal of this cpSegmentShape.",
	"cpSegmentShapeGetRadius" => "Get the radius of this cpSegmentShape.",
	
	"cpShapeGetBB" => "Get the bounding box of this cpShape.",
	"cpShapeGetBody" => "Get the body this cpShape is attached to.",
	"cpShapeGetCollisionType" => "Get the collision type of this cpShape.",
	"cpShapeGetElasticity" => "Get the elasticity of this cpShape.",
	"cpShapeGetFriction" => "Get the friction of this cpShape.",
	"cpShapeGetGroup" => "Get the group of this cpShape.",
	"cpShapeGetLayers" => "Get the layer bitmask of this cpShape.",
	"cpShapeGetSensor" => "Get the sensor flag of this cpShape.",
	"cpShapeGetSurfaceVelocity" => "Get the surface velocity of this cpShape.",
	"cpShapeGetUserData" => "Get the user data pointer of this cpShape.",
	
	"cpSimpleMotorGetRate" => "Get the rate of this cpSimpleMotor.",
	
	"cpSlideJointGetAnchr1" => "Get the anchr1 of this cpSlideJoint.",
	"cpSlideJointGetAnchr2" => "Get the anchr2 of this cpSlideJoint.",
	"cpSlideJointGetMax" => "Get the max distance between the anchors of this cpSlideJoint.",
	"cpSlideJointGetMin" => "Get the min distance between the anchors of this cpSlideJoint.",
	
	"cpSpaceGetCollisionBias" => "Get the collision bias of this cpSpace.",
	"cpSpaceGetCollisionPersistence" => "Get the collision persistence of this cpSpace.",
	"cpSpaceGetCollisionSlop" => "Get the collision slop of this cpSpace.",
	"cpSpaceGetCurrentTimeStep" => "Get the most recent timestep used with this cpSpace.",
	"cpSpaceGetDamping" => "Get the damping of this cpSpace.",
	"cpSpaceGetEnableContactGraph" => "Get the enable contact graph flag of this cpSpace.",
	"cpSpaceGetGravity" => "Get the gravity of this cpSpace.",
	"cpSpaceGetIdleSpeedThreshold" => "Get the idle speed threshold of this cpSpace.",
	"cpSpaceGetIterations" => "Get the number of solver iterations of this cpSpace.",
	"cpSpaceGetSleepTimeThreshold" => "Get the sleep time threshold of this cpSpace.",
	"cpSpaceGetStaticBody" => "Get the static body of this cpSpace.",
	"cpSpaceGetUserData" => "Get the user data pointer of this cpSpace.",
}

def output_getter(func)
	name = func[:name]
	
	doc = GETTER_DOCS[name]
	return if doc == SKIP
	
	struct, property = */(cp\w*)Get(.+)/.match(name).captures
	
	if doc
		prototype = "#{func[:inline] ? "static inline " : ""}#{func[:return]} #{name}(#{func[:args]})"
		
DOC_FILE.puts <<-EOF
/// @addtogroup #{struct}
/// @{
/// @fn #{prototype};
/// @brief #{doc}
#{prototype};
/// @}

EOF
	else
		puts %{\t"#{name}" => "Get the #{property.downcase} of this #{struct}.",}
	end
end


def output_setter(func)
	name = func[:name]
	
	doc = GETTER_DOCS[name.gsub("Set", "Get")]
	return if doc == SKIP
	
	struct, property = */(cp\w*)Set(.+)/.match(name).captures
	
	if doc
		prototype = "static inline void #{name}(#{func[:args]})"
		
DOC_FILE.puts <<-EOF
/// @addtogroup #{struct}
/// @{
/// @fn #{prototype};
/// @brief #{doc.gsub("Get", "Set")}
#{prototype};
/// @}"

EOF
	else
		puts %{\t"#{name}" => "Set the #{property.downcase} of this #{struct}.",}
	end
end


getters = FUNCS.keys.find_all{|name| /(cp\w*)Get(.+)/.match(name)}.sort
FUNCS.values_at(*getters).each{|func| output_getter(func)}

setters = FUNCS.keys.find_all{|name| /(cp\w*)Set(.+)/.match(name)}.sort
FUNCS.values_at(*setters).each{|func| output_setter(func)}

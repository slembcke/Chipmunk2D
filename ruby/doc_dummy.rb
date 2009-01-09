
class Float 
	# Normal floating point infinity. For your convenience.
	INFINITY = 1.0/0.0
end

# Convenience method for CP::Vec2.new(x, y)
def vec2(x, y); end

# Chipmunk Game Dynamics. Provides fast, easy to use, robust physics.
module CP
  # The bias coefficient used for resolving penetrations. Setting 0.0 effectively disables
  # penetration resolution. Setting 1.0 will try to resolve the penetration in a single step.
  # (not recommended) The default is 0.1
  attr_accessor :bias_coef
    
  # The maximum allowed penetration distance. If an object penetrates by more than this
  # value, they will be pushed apart. Setting a non zero value helps to prevent
  # oscillating contacts. Defaults to 0.1, and can be any positive value.
  attr_accessor :collision_slop
    
  # Calculate the moment of inertia for a circle with the given mass,
  # inner and outer radii, and offset. _offset_ should be a CP::Vect.
  #
  # Various other formulas for moments of inertia can be found here: 
  # http://en.wikipedia.org/wiki/List_of_moments_of_inertia
  def moment_for_circle(m, r1, r2, offset); end
  
  # Calculate the moment of inertia for a polygon with the given mass,
  # vertexes, and offset. _verts_ should be an Array of CP::Vect with
  # a counterclockwise winding, _offset_ should be a CP::Vect.
  #
  # Various other formulas for moments of inertia can be found here: 
  # http://en.wikipedia.org/wiki/List_of_moments_of_inertia
  def moment_for_poly(m, verts, offset); end

  # Basic 2D vector class.
  class Vec2
    attr_accessor :x, :y
    
    # Given an angle in radians, it returns a unit length vector.
    def self.for_angle(a); end
    
    def initialize(x, y); end
    
    # Return the vector as an array.
    def to_a; end
    
    def to_s; end
    
    # Get the angle (in radians) the vector is pointing in.
    def to_angle; end

    # Vector negation.
    def -@; end

    # Vector addition.
    def +(vect); end

    # Vector subtraction.
    def -(vect); end

    # Scalar multiplication.
    def *(scalar); end

    # Scalar division.
    def /(scalar); end

    # Vector dot product.
    def dot(vect); end

    # 2D cross product analog.  Z component of the 3D cross product.
    def cross(vect); end

    # Length of the vector.
    def length(vect); end

    # Squared length of the vector. Faster to calculate than the length, and
    # may be used in comparisons.
    def lengthsq(vect); end

    # Normalize a vector.		
    def normalize; end
    
    # Normalize a vector in place.		
    def normalize!; end
    
    # Get the perpendicular vector.
    def perp; end
    
    # Vector projection of self onto vect.
    def project(vect); end
    
    # Rotate and scale self by vect using complex multiplication.
    def rotate(vect); end
    
    # Inverse of Vect#rotate.
    def unrotate(vect); end
    
    # Returns true if self and vect are within dist.
    def near?(vect, dist); end
  end
  
  # Basic 2D bounding box class.
  class BB
    # Left.
    attr_accessor :l

    # Bottom.
    attr_accessor :b

    # Right.
    attr_accessor :r

    # Top.
    attr_accessor :t
    
    # Left, bottom, right and top.
    def initialize(l, b, r, t); end
    
    # Test if the BBs intersect.
    def intersect?(other); end
    
    # Clamps vector _v_ to the nearest point inside bounding box _self_.
    def clamp_vect(v); end
    
    # Wraps vector _v_ to be inside bounding box _self_.
    def wrap_vect(v); end
    
    def to_s; end
  end
  
  # The Chipmunk rigid body class. A rigid body holds the physical properties of an object. 
  # (mass, position, rotation, velocity, etc.) It does not have a shape by itself. 
  # If you’ve done physics with particles before, rigid bodies differ mostly in that 
  # they are able to rotate.
  class Body
    # Mass. Replaces the now deprecated body.m.
    attr_accessor :mass
    
    # Moment of inertia. Replaces the now deprecated body.i.
    attr_accessor :moment
    
    # Position. Replaces the now deprecated body.p.
    attr_accessor :pos
    
    # Velocity. Replaces the now deprecated body.v.
    attr_accessor :vel
    
    # Force. Replaces the now deprecated body.f.
    attr_accessor :force
    
    # Angle. (in radians) Replaces the now deprecated body.a.
    attr_accessor :angle
    
    # Angular Velocity. (in radians/second) Replaces the now deprecated body.w.
    attr_accessor :ang_vel
    
    # Torque. Replaces the now deprecated body.t.
    attr_accessor :torque
    
    # Rotation expressed unit length vector.
    attr_reader :rot
    
    # Covnverts vector _v_ from body local coordinates to world space coordinates.
    def local2world(v); end
    
    # Covnvert vector _v_ from world space coordinates to body local coordinates.
    def world2local(v); end
    
    # Create a body with the given mass and moment of inertia.
    def initialize(m, i); end
    
    # Zero the accumulated forces and torques.
    def reset_forces; end
    
    # Accumulate a force as applied at the offset in world
    # coordinates.
    def apply_force(force, offset); end
    
    # Apply an impulse as applied at the offset in world coordinates.
    def apply_impulse(j, r); end
    
    # Integrate the velocity of the body for the given gravity,
    # damping and time step. (Uses Euler integration)
    def update_velocity(gravity, damping, dt); end

    # Integrate the position of the body for the given time
    # step. (Uses Euler integration)
    def update_position(dt); end
  end
  
  # Collision shapes module. By attaching shapes to bodies, you can define the a 
  # body’s shape. You can attach many shapes to a single body to define a complex 
  # shape, or none if it doesn’t require a shape.
  module Shape
    # The CP::Body the shape is connected to.
    attr_accessor :body
    
    # The collision type for the shape. The actual collision type used
    # is the object id of the object you pass as the collision type,
    # meaning you can use any Ruby object as an identifier. Symbols work well.
    attr_accessor :collision_type
    
    # Shapes in the same non-nil collision group do not create collisions.
    # Any object can be used as a collision identifier.
    attr_accessor :group
    
    # A 32bit bitmask to set which layers the shape should collide in.
    # Shapes only collide with other shapes that share a layer. Each layer
    # is represented by a bit, and the default layer is all of them.
    attr_accessor :layers
    
    # The elasticity of the shape. Using a value of 1.0 or higher is not 
    # recommended. The final elasticity value is the product of the two shapes.
    attr_accessor :e
    
    # The frictional coefficient of the shape. A value of 1.0 represents
    # an object that will not slide on a 45 degree angle. The final frictional
    # coefficient is the product of the two shapes.
    attr_accessor :u
    
    # The surface velocity is used by the friction calculations.
    # You can use this to make conveyour belts or characters that walk.
    # Interpreted in world space.
    attr_accessor :surface_v
    
    # Delegate object. Useful from within collision pair functions.
    # Meant as a convenience so that you don't need to subclass the
    # shape classes for trivial uses.
    attr_accessor :obj
    
    # Circle collision shape.
    class Circle
			include Shape
			
			# Create a circle collision shape attached to the given body at
			# the given offset with the given radius.
			def initialize(body, radius, offset); end
    end
    
    # Segment collision shape.
    class Segment
			include Shape
			
			# Create a segment collision shape attached to the given body
			# with endpoints _a_ and _b.
			def initialize(body, a, b); end
    end
    
    # Poly collision shape.
    class Poly
			include Shape
			
			# Create a poly collision shape attached to the given body at
			# the given offset with the given vertexes. _verts_ must be an
			# Array of CP::Vect with a counterclockwise winding.
			def initialize(body, verts, offset); end
    end
  end
  
  # Constraints module. Defined herein are numerous joints, gears, and other
  # constraints that act on pairs of bodies to constrain their motion.
  module Constraint
  
  # Read the first body used in the constraint
  attr_reader :body_a

  # Read the first body used in the constraint
  attr_reader :body_b
  
  # The maximum amount of force that can be applied to correct the bodies in the joint.
  # Defaults to infinity.
  attr_accessor :max_force
  
  # The percentage of deformation that is being corrected per tick. Defaults to 0.1
  attr_accessor :bias_coef
  
  # The rate at which the deformation is corrected per unit time. For example
  # a value of 100 will move the objects up to 100 units per unit time.
	# Defaults to infinity.
  attr_accessor :max_bias
    
    # A solid pin or rod connecting two rigid bodies.
    class PinJoint
			# Creates a joint between the given bodies with the given
			# anchors. Anchor points are in body relative coordinates, where
			# anchor1 is relative to body1, and anchor2 is relative to body2.
			# The joint length defaults to the distance between the
			# anchors at creation time.
			def initialize(body1, body2, anchor1, anchor2); end
			
			attr_accessor :anchr1
			attr_accessor :anchr2
			attr_accessor :dist
    end
    
    # Like a pin joint, but with a variable length.
    class SlideJoint
			# Creates a joint between the given bodies with the given
			# anchors and minimum and maximum anchor distances.
			def initialize(body1, body2, anchor1, anchor2, min, max); end

			attr_accessor :anchr1
			attr_accessor :anchr2
			attr_accessor :min
			attr_accessor :max
    end
    
    # The bodies pivot about a single point.
    class PivotJoint
			# Creates a pivot between the given bodies at the given anchor
			# point. _pivot_ should be in world coordinates.
			def initialize(body1, body2, pivot); end

			attr_accessor :anchr1
			attr_accessor :anchr2
    end
    
    # One body has a pivot that connects to a linear groove in the other.
    class GrooveJoint
    	# _grv_a_ and _grv_b_ define the groove on body1_ and _anchr2_
    	# defines the anchor on body2. All coordinates in body local coordinates.
    	def initialize(body1, body2, grv_a, grv_b, anchr2); end
    end
    
    # Like a torsion spring. http://en.wikipedia.org/wiki/Torsion_spring
    class DampedRotarySpring
    	
    	# Rest angle is the desired angular offset between body1 and body2
    	# calculated as body2.angle - body1.angle.
    	# Stiffness is similar to Young's modulus but uses torque instead of force
    	# Damping is described here: http://en.wikipedia.org/wiki/Damping
    	def initialize(body1, body2, restAngle, stiffness, damping); end
    	
			attr_accessor :rest_angle
			attr_accessor :stiffness
			attr_accessor :damping
    end
   
    # A normal spring. Seeks to be at a certain length.
    class DampedSpring
    	
    	# Rest length is the desired length of the spring.
    	# Stiffness is similar to Young's modulus but uses torque instead of force
    	# Damping is described here: http://en.wikipedia.org/wiki/Damping
    	def initialize(body1, body2, anchor1, anchor2, restLength, stiffness, damping); end
    	
			attr_accessor :anchr1
			attr_accessor :anchr2
			attr_accessor :restLength
			attr_accessor :stiffness
			attr_accessor :damping
    end
    
    # Defines direct relationships between rotation of two bodies. Has
    # many relationships beyond just simple gears, by adjusting the
    # default parameters.
    class GearJoint
			# Phase represents angular offset of the body2 from body1 when
			# body1 is at an angle of 0.
			# Ratio represents the gear ratio between the two.
			def initialize(body1, body2, phase, ratio); end

			attr_accessor :phase
			attr_accessor :ratio
    end
    
    # Limits the rotation of one body relative to another. Min and max
    # ranges can be less than zero or greater than 2*Pi, if you want
    # an object to rotate several times before hitting the angular limit
    class RotaryLimitJoint
			# The min and max rotation is specified in radians.
			def initialize(body1, body2, min, max); end

			attr_accessor :min
			attr_accessor :max
    end
    
    # Causes the bodies to rotate at a specified rate, relative to eachother.
    # This applies whatever torque needed to cause that rotation, so you'll 
    # likely limit it with max_force.
    class SimpleMotor
			# Create a motor with the specified rate in radians per unit time.
			def initialize(body1, body2, rate); end

			attr_accessor :rate
    end
  end
  
  # The Chipmunk space class. Spaces are the basic simulation unit in Chipmunk. 
  # You add bodies, shapes and joints to a space, and then update the space as a whole.
  class Space
    # The number of iterations to use when solving
    # constraints. (collisions and joints)
    attr_accessor :iterations
    
    # The amount of damping to apply to the system when updating.
    attr_accessor :damping
    
    # The amount of gravity in the system. Must be a CP::Vect.
    attr_accessor :gravity
    
    # Iterations used when solving for elasticity.
    attr_accessor :elastic_iterations
    
    def initialize; end
    
    # Add a block to be called when a collision between a shape with a
    # _collision_type_ of _type_a_ and a shape with a _collision_type_
    # of _type_b_ is found. Supplying no block or _&nil_ will reject
    # any collision between the two given collision types.
    def add_collision_func(type_a, type_b, &block); end
    
    # Remove a block added with _add_collision_func().
    def remove_collision_func(type_a, type_b); end

    # Set the default collision function to be used when no specifid
    # function is found. Normally this simply accepts all
    # collisions. Supplying no block or _&nil_ will reject collisions
    # by default. Keep in mind that the default func will handle a lot
    # of collisions, and you don't want 100,000 block calls a second!
    def set_default_collision_func(&block); end

    # Set the default collision function. This block will be called
    # whenever an unhandled collision pair is found. The default
    # behavior is to accept all collisions, passing no block or _&nil_
    # will reject all collisions by default.
    
    # Add the given shape to the space's active spatial hash. Shapes
    # attached to moving bodies should be added here as they will be
    # rehashed on every call to Space#step
    def add_shape(shape); end
    
    # Add the given shape to the space's static spatial hash. Static
    # shapes are only rehashed when Space#rehash_static is called, so
    # they should not move.
    def add_static_shape(shape); end
    
    # Add the given rigid body to the space.
    def add_body(body); end
    
    # Add the given constraint to the space.
    def add_constraint(constraint); end

    # Remove the given joint from the space's static spatial hash.
    def remove_shape(shape); end
    
    # Remove the given shape from the space's active spatial hash.
    def remove_static_shape(shape); end
    
    # Remove the given body from the space.
    def remove_body(body); end
   
    # Remove the given constraint from the space.
    def remove_constraint(constraint); end
    
    # Resize the static spatial hash.
    def resize_static_hash(dim, count); end
    
    # Resize the active spatial hash.
    def resize_active_hash(dim, count); end
    
    # Calls the block for every shape that contains the point.
    def shape_point_query(point); yield(shape); end
    
    # Calls the block for every static shape that contains the point.
    def static_shape_point_query(point); yield(shape); end
    
    # Rehash the static spatial hash.
    def rehash_static; end
    
    # Move the space forward by _dt_ seconds. Using a fixed size time
    # step is highly recommended for efficiency of the contact
    # persistence algorithm.
    def step(dt); end
  end
end


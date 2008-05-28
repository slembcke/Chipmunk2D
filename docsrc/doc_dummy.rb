# Chipmunk Game Dynamics. Provides fast, easy to use, robust physics.
module CP
  # The bias coefficient used for resolving penetrations. Setting 0.0 effectively disables
  # penetration resolution. Setting 1.0 will try to resolve the penetration in a single step.
  # (not recommended)
  attr_accessor :bias_coef
    
  # The maximum allowed penetration distance. Setting a non zero value helps to prevent
  # oscilating contacts.
  attr_accessor :collision_slop
    
  # Calculate the moment of inertia for a circle with the given mass,
  # inner and outer radii, and offset. _offset_ should be a CP::Vect.
  def moment_for_circle(m, r1, r2, offset); end
  
  # Calculate the moment of inertia for a polygon with the given mass,
  # vertexes, and offset. _verts_ should be an Array of CP::Vect with
  # a counterclockwise winding, _offset_ should be a CP::Vect.
  def moment_for_poly(m, verts, offset); end
  
  # Accumulate forces on the given bodies by connecting a spring
  # between the anchors. Body local coordinates are used for the
  # anchor locations.
  def damped_spring(body1, body2, anchor1, anchor2, rest_length, spring, damping, dt); end

  # Basic 2D vector class.
  class Vect
    attr_accessor :x, :y
    
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

    # Squared ength of the vector.
    def lengthsd(vect); end

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
    
    # Clamps _v_ to _self_.
    def clamp_vect(v); end
    
    # Wraps _v_ to _self_.
    def wrap_vect(v); end
    
    def to_s; end
  end
  
  # The Chipmunk rigid body class.
  class Body
    # Mass.
    attr_accessor :m
    
    # Moment of inertia.
    attr_accessor :i
    
    # Position.
    attr_accessor :p
    
    # Velocity.
    attr_accessor :v
    
    # Force.
    attr_accessor :f
    
    # Angle. (in radians)
    attr_accessor :a
    
    # Angular Velocity. (in radians/second)
    attr_accessor :w
    
    # Torque.
    attr_accessor :t
    
    # Rotation vector.
    attr_reader :rot
    
    # Covnvert body local coordinates to world space coordinates.
    def local2world(v); end
    
    # Covnvert world space coordinates to body local coordinates.
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
    def update_velocity(gravity, damping, dt); end
  end
  
  # Collision shapes module.
  module Shape
    # The CP::Body the shape is connected to.
    attr_accessor :body
    
    # The collision type for the shape. The actual collision type used
    # is the object id of the object you pass as the collision type,
    # meaning you can use any Ruby object as an identifier.
    attr_accessor :collision_type
    
    # Shapes in the same non-nil collision group do not create collisions.
    # Any object can be used as a collision identifier.
    attr_accessor :group
    
    # A 32bit bitmask to set which layers the shape should collide in.
    attr_accessor :layers
    
    # The elasticity of the shape.
    attr_accessor :e
    
    # The frictional coefficient of the shape.
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
      # with endpoints _a_ and _b_.
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
  
  # Joints module.
  module Joint
    # A solid pin or rod connecting two rigid bodies.
    class Pin
      # Creates a joint between the given bodies with the given
      # anchors. The joint length is set to the distance between the
      # anchors at creation time.
      def initialize(body1, body2, anchor1, anchor2); end
    end
    
    # Like a pin joint, but with a variable length.
    class Slide
      # Creates a joint between the given bodies with the given
      # anchors and minimum and maximum anchor distances.
      def initialize(body1, body2, anchor1, anchor2, min, max); end
    end
    
    # The bodies pivot about a single point.
    class Pivot
      # Creates a pivot between the given bodies at the given anchor
      # point. _pivot_ should be in world coordinates.
      def initialize(body1, body2, pivot); end
    end
    
    # One body has a pivot that connects to a linear groove in the other.
    class Groove
    	# _grv_a_ and _grv_b_ define the groove on _body1_ and _anchr2_
    	# defines the anchor on _body2_. All coordinates in body local coordinates.
    	def initialize(body1, body2, grv_a, grv_b, anchr2); end
    end
  end
  
  # The Chipmunk space class.
  class Space
    # The number of iterations to use when solving
    # constraints. (collisions and joints)
    attr_accessor :iterations
    
    # The amount of damping to apply to the system when updating.
    attr_accessor :damping
    
    # The amount of gravity in the system. Must be a CP::Vect.
    attr_accessor :gravity
    
    def initialize; end
    
    # Add a block to be called when a collision between a shape with a
    # _collision_type_ of _type_a_ and a shape with a _collision_type_
    # of _type_b_ is found. Supplying no block or _&nil_ will reject
    # any collision between the two given collision types.
    def add_collision_func(type_a, type_b, &block); end
    
    # Remove a block added with _add_collision_func()_.
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
    
    # Add the given joint to the space.
    def add_joint(joint); end

    # Remove the given joint from the space's static spatial hash.
    def remove_shape(shape); end
    
    # Remove the given shape from the space's active spatial hash.
    def remove_static_shape(shape); end
    
    # Remove the given body from the space.
    def remove_body(body); end
    
    # Remove the given joint from the space.
    def remove_joint(joint); end
    
    # Resize the static spatial hash.
    def resize_static_hash(dim, count); end
    
    # Resize the active spatial hash.
    def resize_active_hash(dim, count); end
    
    # Rehash the static spatial hash.
    def rehash_static; end
    
    # Move the space forward by _dt_ seconds. Using a fixed size time
    # step is highly recommended for efficiency of the contact
    # persistence algorithm.
    def step(dt); end
  end
end

# Creates a new CP::Vect
def vec2(x, y); end

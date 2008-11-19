module CP
	module Object
		attr_reader :chipmunk_objects
		
		def chipmunk_objects
			if @chipmunk_objects
				return @chipmunk_objects
			else
				raise "This CP::Object (#{self.class}) did not call #init_chipmunk_object."
			end
		end
		
		private
		def init_chipmunk_object(*objs)
			bad_objs = objs.reject{|obj| obj.is_a?(CP::Object)}
			raise(ArgumentError, "The following objects: #{bad_objs.inspect} are not CP::Objects") unless bad_objs.empty?

			@chipmunk_objects = objs.inject([]){|sum, obj| sum + obj.chipmunk_objects}.uniq
		end
	end
	
	class Body
		include CP::Object

		def chipmunk_objects
			[self]
		end
		
		def add_to_space(space)
			space.add_body(self)
		end
		
		def remove_from_space(space)
			space.remove_body(self)
		end
	end
	
	module Shape
		include CP::Object
		
		def chipmunk_objects
			[self]
		end
		
		def add_to_space(space)
			space.add_shape(self)
		end
		
		def remove_from_space(space)
			space.remove_shape(self)
		end
	end
	
	module Constraint
		include CP::Object
		
		def chipmunk_objects
			[self]
		end
		
		def add_to_space(space)
			space.add_constraint(self)
		end
		
		def remove_from_space(space)
			space.remove_constraint(self)
		end
	end
	
	class Space
		def add_object(obj)
			obj.chipmunk_objects.each{|elt| elt.add_to_space(self)}
		end
		
		def remove_object(obj)
			obj.chipmunk_objects.each{|elt| elt.remove_from_space(self)}
		end
	end
end

require 'chipmunk'

# match 0 is the whole function proto
# match 1 is either "static inline " or nil
# match 2 is the return type
# match 3 is the function symbol name
# match 4 is the arguments
PATTERN = /.*?((static inline )?(\w*\*?)\s(cp\w*)\((.*?)\))/

IO.readlines("|gcc -DNDEBUG -E ../include/chipmunk/chipmunk.h").each do|line|
	str = line
	
	while match = PATTERN.match(str)
		str = match.post_match
		
		proto, inline, ret, name, args = match.captures.values_at(0, 1, 2, 3, 4)
		next if ret == "return" || ret == ""
		
		inline = !!inline
		
		p({:inline => inline, :return => ret, :name => name, :args => args})
#		puts "#{name} - #{inline ? "static inline " : ""}#{ret} #{name}(#{args})"
	end
end

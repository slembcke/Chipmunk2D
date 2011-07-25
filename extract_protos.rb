# extract function prototypes from the headers for making FFIs, etc.
# use like this: find include/ -name "*.h" | xargs cat | ruby extract_protos.rb

FORMAT = ARGV[0]

# match 0 is the whole function proto
# match 1 is the return value
# match 2 returns pointer?
# match 3 is the function name
# match 4 is the remainder
REGEX = /.*?((\w+(\s+\*)?)\s*(cp\w*)\(.*?\))(.*)/

STDIN.readlines.each do|str|
	while match = REGEX.match(str)
		captures = match.captures
		proto, ret, sym, str = captures.values_at(0, 1, 3, 4)
		break if ret == "return"
		
		if FORMAT
			puts eval('"' + FORMAT + '"')
		else
			puts "* #{proto} - #{sym}"
		end
	end
end

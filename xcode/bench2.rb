results = []

cmd = "|pbpaste"
open(cmd) do|io|
	while str = io.gets do
		puts str
		results << str.split[2]
	end
end

puts "Placing results in paste buffer."
open("|pbcopy", "w"){|io| io.puts results}

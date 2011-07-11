DOC_FILE = open("generated_docs", "w")

def output(str='')
	DOC_FILE.puts str
end

def find_declarations(name)
	IO.readlines("|find include -name \"*.h\" | xargs grep -h \"^#{name}.*;\\s*$\"")
end

def output_getter(struct_base, struct, type, name)
	output "/// @fn static inline #{type} #{struct}Get#{name}(const #{struct_base} *#{struct_base[2..-1].downcase});"
	output "/// Get the #{name} of a #{struct}."
	output
end

def output_setter(struct_base, struct, type, name)
	output "/// @fn static inline void #{struct}Set#{name}(#{struct_base} *#{struct_base[2..-1].downcase}, #{type} value);"
	output "/// Set the #{name} of a #{struct}."
	output
end

def struct_parser(decl)
	decl.match(/\((.*?), (.*?), (.*?)(, .*)?\)/).captures.values_at(0, 2)
end

def search_struct(struct)
	upname = struct[2..-1].capitalize
	find_declarations("CP_Define#{upname}StructProperty").each do|decl|
		type, name = *struct_parser(decl)
		output_getter(struct, struct, type, name)
		output_setter(struct, struct, type, name)
	end
	
	find_declarations("CP_Define#{upname}StructGetter").each do|decl|
		output_getter(struct, struct, *struct_parser(decl))
	end
	
	find_declarations("CP_Define#{upname}StructSetter").each do|decl|
		output_setter(struct, struct, *struct_parser(decl))
	end
end

search_struct("cpBody")
search_struct("cpShape")
search_struct("cpConstraint")
search_struct("cpSpace")

def output_getter2(struct_base, struct, type, name)
	output "/// @addtogroup #{struct}"
	output "/// @{"
	output "/// @fn static inline #{type} #{struct}Get#{name}(const #{struct_base} *#{struct_base[2..-1].downcase});"
	output "/// @brief Get the #{name} of a #{struct}."
	output "static inline #{type} #{struct}Get#{name}(const #{struct_base} *#{struct_base[2..-1].downcase});"
	output "/// @}"
	output
end

def output_setter2(struct_base, struct, type, name)
	output "/// @addtogroup #{struct}"
	output "/// @{"
	output "/// @fn static inline void #{struct}Set#{name}(#{struct_base} *#{struct_base[2..-1].downcase}, #{type} value);"
	output "/// @brief Set the #{name} of a #{struct}."
	output "static inline void #{struct}Set#{name}(#{struct_base} *#{struct_base[2..-1].downcase}, #{type} value);"
	output "/// @}"
	output
end

def struct_parser2(decl)
	decl.match(/\((.*?), (.*?), (.*?), (.*?)\)/).captures.values_at(0, 1, 3)
end

def search_struct2(struct_base)
	upname = struct_base[2..-1].capitalize
	find_declarations("CP_\\(Define\\|Declare\\)#{upname}Property").each do|decl|
		struct, type, name = *struct_parser2(decl)
		output_getter2(struct_base, struct, type, name)
		output_setter2(struct_base, struct, type, name)
	end
	
	find_declarations("CP_Define#{upname}Getter").each do|decl|
		output_getter2(struct_base, *struct_parser2(decl))
	end
	
	find_declarations("CP_Define#{upname}Setter").each do|decl|
		output_setter2(struct_base, *struct_parser2(decl))
	end
end

search_struct2("cpConstraint")

def output_getter2(struct_base, struct, type, name)
	output "/// @fn #{type} #{struct}Get#{name}(const #{struct_base} *#{struct_base[2..-1].downcase});"
	output "/// Get the #{name} of a #{struct}."
	output
end

def output_setter2(struct_base, struct, type, name)
	output "/// @fn void #{struct}Set#{name}(#{struct_base} *#{struct_base[2..-1].downcase}, #{type} value);"
	output "/// Set the #{name} of a #{struct}."
	output
end

def struct_parser3(decl)
	decl.match(/\((.*?), (.*?), (.*?)\)/).captures
end

def search_struct3(struct_base)
	upname = struct_base[2..-1].capitalize
	find_declarations("CP_\\(Define\\|Declare\\)#{upname}Property").each do|decl|
		struct, type, name = *struct_parser3(decl)
		output_getter2(struct_base, struct, type, name)
		output_setter2(struct_base, struct, type, name)
	end
	
	find_declarations("CP_\\(Define\\|Declare\\)#{upname}Getter").each do|decl|
		output_getter2(struct_base, *struct_parser3(decl))
	end
	
	find_declarations("CP_\\(Define\\|Declare\\)#{upname}Setter").each do|decl|
		output_setter2(struct_base, *struct_parser3(decl))
	end
end

search_struct3("cpShape")

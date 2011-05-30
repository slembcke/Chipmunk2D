require 'rubygems'
require 'redcloth'
require 'erb'

class Node
	attr_reader :anchor
	attr_reader :children
	
	def initialize(title, anchor, parent)
		@title = title
		@anchor = (parent.anchor ? "#{parent.anchor}-#{anchor}" : anchor)
		
		@children = []
		parent.children << self
	end
	
	def outline(level)
		stars = "*"*level
		children = @children.map{|child| child.outline(level + 1)}
		(["#{stars} \"#{@title}\":##{anchor}"] + children).join("\n")
	end
	
	Root = Struct.new(:anchor, :children).new(nil, [])
	def Root.outline
		self.children.map{|child| child.outline(1)}.join("\n")
	end
end

def pop_open_div(name)
	return <<-EOF
		<a class="HideShow" href="javascript:;" onmousedown="toggleDiv('Pop Open #{name}');">Hide/Show #{name}</a>
		<div class="PopOpen" id="Pop Open #{name}" style="display:none">
	EOF
end

$node_path = [Node::Root]

def h(level, title, anchor)
	$node_path = $node_path[0, level]
	parent = $node_path.last
	
	node = Node.new(title, anchor, parent)
	$node_path << node
	
	"<a name=\"#{node.anchor}\" /><h#{level}><a href=\"##{node.anchor}\">#{title}</a></h#{level}>"
end

textile = ERB.new(File.read('chipmunk-docs.textile')).result(binding)
textile_with_outline = ERB.new(textile).result(binding)
redcloth_out = RedCloth.new(textile_with_outline).to_html

html = <<HTML
	<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
	<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
		<head>
			<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
			<title>Chipmunk Game Dynamics Documentation</title>
			<link rel="stylesheet" type="text/css" href="stylesheet.css" />
		</head>
		<body>
			<script language="javascript">
				function toggleDiv(divid){
					if(document.getElementById(divid).style.display == 'none'){
						document.getElementById(divid).style.display = 'block';
					}else{
						document.getElementById(divid).style.display = 'none';
					}
				}
			</script>
			
			#{redcloth_out}
		</body>
	</html>
HTML

File.open('../index.html', 'w') do|f|
	f.write html
end

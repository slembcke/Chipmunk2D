require 'rubygems'
require 'redcloth'
require 'erb'
require 'uri/common'


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
		children = ""
		if level == 1
			children = "<ul>#{@children.map{|child| child.outline(level + 1).join}}</ul>"
		end
		
		["<li><a href=\"##{anchor}\">#{@title}</a>#{children}</li>"]
	end
	
	Root = Struct.new(:anchor, :children).new(nil, [])
	def Root.outline
		"<ul>#{self.children.map{|child| child.outline(1)}.join}</ul>"
	end
end

def pop_open_div(name)
	return %{<div><a class="HideShow" href="javascript:;" onmousedown="toggleDiv('Pop Open #{name}');">Hide/Show #{name}</a><div class="PopOpen" id="Pop Open #{name}" style="display:none">}
end

def pop_open_example(name)
	divid = "Pop Open #{name} Example"
	return %{<div><a class="HideShow" href="javascript:;" onmousedown="toggleExample('#{divid}', 'examples/#{URI.escape(name)}.html');">Hide/Show #{name} Example</a><div id="#{divid}" style="display:none"></div></div>}
end

$node_path = [Node::Root]

def h(level, title, anchor)
	$node_path = $node_path[0, level]
	parent = $node_path.last
	
	node = Node.new(title, anchor, parent)
	$node_path << node
	
	"<h#{level}><a name=\"#{node.anchor}\"></a><a href=\"##{node.anchor}\">#{title}</a></h#{level}>"
end

# Run the ERB once to grab the outline information and insert headers.
textile = ERB.new(File.read('chipmunk-docs.textile')).result(binding)
textile2 = ERB.new(textile).result(binding)
redcloth_out = RedCloth.new(textile2).to_html

html = <<HTML
	<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
	<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
		<head>
			<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
			<title>Chipmunk Game Dynamics Manual</title>
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
				
				function toggleExample(divid, name){
					toggleDiv(divid);
					var div = document.getElementById(divid);
					var i = div.innerHTML;
					
					if(div.innerHTML == ""){
						div.innerHTML = "Loading ..."
						
						var xmlhttp = new XMLHttpRequest();
						xmlhttp.onreadystatechange = function(){
							if(xmlhttp.readyState == 4){
								div.innerHTML = xmlhttp.responseText;
							}
						}
						
						xmlhttp.open("GET", name, true);
						xmlhttp.send();
					}
				}
			</script>
			
			#{redcloth_out}
		</body>
	</html>
HTML

File.open('../doc/index.html', 'w') do|f|
	f.write html
end

require 'rubygems'
require 'redcloth'

html = <<HTML
	<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
	<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
		<head>
			<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
			<title>Chipmunk Game Dynamics Documentation</title>
						<style type="text/css">
							h1, h2 { padding: 3px; }
							h1 { background-color: #DB944D;}
							h2 { background-color: lightGrey;}
							p { margin-left: 1em; }
							p.expl { margin-left: 2em; }
							code { color: #191970 }
							
							pre {
									background-color: #F0F0F0;
									padding: 3px;
									margin-left: 1em;
							}
							
							table {
								border: 2px solid black;
								border-collapse: collapse;
								margin-left: 1em;
							}
							
							table td, th {
								border: 1px black solid;
								padding: 2px;
							}
					</style>
		</head>
		<body>
			#{RedCloth.new(File.read('chipmunk-docs.textile')).to_html}
		</body>
	</html>
HTML

File.open('../chipmunk-docs.html', 'w') do|f|
	f.write html
end

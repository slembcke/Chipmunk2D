require 'rubygems'
require 'redcloth'

html = <<HTML
	<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
	<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
		<head>
			<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
			<title>Chipmunk Game Dynamics Documentation</title>
			<link rel="stylesheet" type="text/css" href="stylesheet.css" />
		</head>
		<body>
			#{RedCloth.new(File.read('chipmunk-docs.textile')).to_html}
		</body>
	</html>
HTML

File.open('../index.html', 'w') do|f|
	f.write html
end

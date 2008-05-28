require 'rubygems'
require_gem 'RedCloth'

cdocs = 'chipmunk-docs'

rc_cdocs = RedCloth.new(File.read(cdocs + '.textile'))

html_cdocs = <<HTML
	<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
	<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
	#{rc_cdocs.to_html}
HTML

File.open("../#{cdocs}.html", 'w') do|f|
	f.write html_cdocs
end

system 'rdoc doc_dummy.rb'
system 'mv doc ../ruby/doc'

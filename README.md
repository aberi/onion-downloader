	Usage: ondl <URL> [-o output_file] [-r] [-O]
To find all the links in the file "index.html" run the command
	parse_test

The parser doesn't work when the buffer only contains some part of the link
(surprise, surprise). Also, if the img tag isn't terminated, it won't
recognize it. That's why these are so difficult to write.

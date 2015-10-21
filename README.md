	Usage: ondl <URL> [-o output_file] [-r] [-O]
To find all the links in the file "index.html" run the command
	
	./parse_test

The onion downloader is a Poor Man's GNU Wget. I wrote it purely as an exercise. It only
works on websites that have very XHTML-like HTML because the parser was written in a few days
and isn't really that smart.

	ondl www.ucla.edu -r
	
will download the homepage of www.ucla.edu as index.html and will download all of the absolute links within
that page and place them in the appropriate directory structure. The -r option turns on recursion, which
can currently only go one level deep. 

Use the options -o or --output-file= to specify an output file. Use --server-response to show the server response.

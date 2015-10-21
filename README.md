	Usage: ondl <URL> [-o output_file] [-r] [-O]
To find all the links in the file "index.html" run the commands
	
	make parse_test
	./parse_test

The onion downloader is a Poor Man's GNU Wget. I wrote it purely as an exercise. It only
works on websites that have very XHTML-like HTML because the parser was written in a few days
and isn't really that smart.

Certain websites that I have found work very well: www.ucla.edu, dartmouth.edu, www.aes.org, hackduke.org. If the
parser fails, sometimes there will be a huge memory leak, so be ready to kill the program at any moment if you aren't
downloading one of these sites (a GB of memory fills up in a second or so). cs.umd.edu can be very slow.

	ondl http://www.aes.org -r # downloads www.aes.org and saves it as index.html. Downloads all links found within the homepage that are on the host www.aes.org
	ondl http://hackduke.org -r --show-response # download all the links on the page + show what response the server gave us (e.g. connection type, cookies, etc.)
	ondl http://cs.umd.edu/index.php --output-file=index.html # save as ./index.html instead of ./index.php
	ondl http://cs.umd.edu/class/fall2015/cmsc351/hwk1.pdf # saves output file to ./hwk1.pdf
	ondl http://cs.umd.edu/class/fall2015/cmsc351/hwk1.pdf -r # saves output file to ./cs.umd.edu/class/fall2015/cmsc351/hwk1.pdf


Use the options -o or --output-file= to specify an output file. Use --server-response to show the server response.

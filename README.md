# onion-downloader
The Onion Downloader is a poor man's GNU Wget. I'm writing it purely as an exercise to learn
how to do network programming and to learn about HTTPS/SSL/TLS and Tor.

It's called the Onion Downloader because one day in the future
it will be able to make requests throught the Tor network to '.onion' sites (as well 
as SSL/TLS support, POST requests, arbitrary recursion, which is currently limited to one
level, cookies, local link conversion for offline viewing, etc.). This would allow a user
to mirror a Tor hidden service, for example, in the same way that a user might create a
site mirror by using wget and its '-M' option. 

Compilation is as simple as running the command
	
	% make

Here are some examples:
	
	Usage: ondl <URL> [-o | --output-file <output_file> ] [-r] [-R | --show-response] 


	% ./ondl http://www.aes.org -r # downloads www.aes.org and saves it as index.html. Downloads all links found within the homepage that are on the host www.aes.org
	
	% ./ondl aes.org -r # Redirects to new location www.aes.org. Same result as previous example.

	% ./ondl http://hackduke.org -r --show-response # download all the links on the page + show what response the server gave us (e.g. connection type, cookies, etc.)
	
	% ./ondl http://cs.umd.edu/index.php --output-file=index.html # save as ./index.html instead of ./index.php
	
	% ./ondl http://cs.umd.edu/class/fall2015/cmsc351/hwk1.pdf # saves output file to ./hwk1.pdf
	
	% ./ondl http://cs.umd.edu/class/fall2015/cmsc351/hwk1.pdf -r # saves output file to ./cs.umd.edu/class/fall2015/cmsc351/hwk1.pdf

	% ./ondl http://crypto.stackexchange.com -r # Will eventually prompt a 503 status code from the server because of too many requests sent. Same IP address as any other Stackexchange site (e.g. StackOverflow)


Use the options -o or --output-file= to specify an output file. Use --server-response to show the server response. -r specifies
to recurse one level, i.e., download every link within the downloaded page that points to a page on the same remote host (for now,
links to other remote hosts are ignored). TODO: Implement recursion for an arbitrary depth.

The 'infinite\_recursion' branch is where I am working on using a queue to 
allow for an arbitrary recursion depth. The master branch only allows for 
recursion depth of 1.

One major TODO is to use an HTML parsing engine instead of trying to write a naive one myself... It's dirty but somehow does a B+ job.

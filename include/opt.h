#ifndef _OPT_
#define _OPT_

struct opt {
	char *url;
	int port;	
	int recursive;
	char *path;	
	char *output_file;
	char *log_file;
	char *method;
	int sock;
	int output_fd;
	int show_server_response;
	int print_content;
};

extern struct opt options;

#endif

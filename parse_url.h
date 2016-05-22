struct url_parts_t {
	char *scheme;
	char *user;
	char *pass;
	char *host;
	unsigned short port;
	char *path;
	char *query_string;
	char *fragment;
};

struct url_parts_t* parse_url(char *url);

// mrsilvers@163.com
// 2020-05-05 17:08:11

struct header{
	char *origin_header;
	char method[8];
	char *uri;
};

struct get{
	char *uri_path;
	char *uri_query;
};

struct post{
	int content_length;
	char *from_data;
	char *uri;
};

struct method{
	struct get get;
	struct post post;
};

struct request{
	char method[8];
	struct header header;
	struct get get;
    struct post post;
};

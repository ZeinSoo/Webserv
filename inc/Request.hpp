#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webServ.hpp"
#include <sys/stat.h>

class Request {

public:
	Request(std::string &requestReceived);
	~Request();

	void parseRequest();
	int parse_uri_path(std::vector<t_routing_info> locations);
	const std::string getMethod();

	const std::string	get_uri_location();
	s_routing_info 		get_location_struct();
	bool getValid();

	int is_CGI();
	void handle_CGI();
	void send_cgi_response(std::string cgi_response);

	void handle_get(int client_sock, double client_max_b_size);
	void		setFd(int client_fd);

	std::map<std::string, std::string> files;
	void sendError(int errorCode);
	void isRequestValid(double client_max_b_size);
	void set_request_server(t_serverConfig *address);
	std::string	get_upload_dir();
	void	set_upload_dir(std::string upload_dir);
	void set_abs_root_path(std::string root_path);
	std::string get_abs_root_path();
	void	handle_delete(int client_sock);
	void	setErrorPage(std::vector<int> error_page);
	ssize_t		getSendValue();

private:
	std::string _requestReceived;
	//request line
	int			_client_fd;
	std::string _method;
	std::string _URI;
	std::string _version;
	std::string _script_filename;
	std::string _query_string;
	std::string _file_content;
	t_routing_info _uri_location;
	t_serverConfig *_request_server;
	std::string _uri_rooted;
	bool _valid;
	struct stat _file;
	std::string _upload_dir;
	std::string _abs_root_path;
	ssize_t _sendValue;

	//headers
	std::map<std::string, std::string> _headers;

	//POST
	std::string _path;
	std::string _body;

	std::vector<int> _error_page;

	// Check Functions

	int checkMethod(std::map<std::string, bool> &methodAvaible, std::string &method);
	int checkURI(std::string &filePath);
};

int	ends_with(const std::string &str, const std::string &suffix);


#endif //REQUEST_HPP

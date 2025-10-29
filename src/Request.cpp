#include "../inc/webServ.hpp"
#include "../inc/Request.hpp"
#include <dirent.h>
#include <Response.hpp>

Request::Request(std::string &requestReceived) : _requestReceived(requestReceived), _valid(true) {
}

Request::~Request() {}

#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <map>

void Request::set_request_server(t_serverConfig *address)
{
	_request_server = address;
}

std::string	Request::get_upload_dir()
{
	return this->_upload_dir;
}

void	Request::set_upload_dir(std::string upload_dir)
{
	_upload_dir = upload_dir;
}

std::string	Request::get_abs_root_path()
{
	return this->_abs_root_path;
}

void	Request::set_abs_root_path(std::string root_path)
{
	_abs_root_path = root_path;
}

s_routing_info Request::get_location_struct()
{
	return this->_uri_location;
}

void Request::parseRequest() {
	_sendValue = 1;
	size_t header_end = _requestReceived.find("\r\n\r\n");
	std::string header_part = _requestReceived.substr(0, header_end);
	std::string body_part = _requestReceived.substr(header_end + 4); 

	std::stringstream ss(header_part);
	std::string method, uri, version;

	ss >> method >> uri >> version;

	_method = method;
	_URI = uri;
	_version = version;

	size_t pos = uri.find('?');
	if (pos != std::string::npos)
	{
		_script_filename = uri.substr(0, pos);
		_query_string = uri.substr(pos + 1);
	} else {
		_script_filename = uri;
		_query_string = "";
	}

	// Parse headers
	std::string header;
	std::string key, value;
	std::getline(ss, header);
	while (std::getline(ss, header)) {
		if (!header.empty() && header[header.size() - 1] == '\r')
			header.erase(header.size() - 1);

		size_t sep = header.find(": ");
		if (sep != std::string::npos)
		{
			key = header.substr(0, sep);
			value = header.substr(sep + 2);
			_headers[key] = value;
		}
	}
	// Capture body
	if (_method == "POST" && _headers.count("Content-Length")) {
		int content_length = std::atoi(_headers["Content-Length"].c_str());

		if ((int)body_part.size() >= content_length)
			_body = body_part.substr(0, content_length);
		else
			std::cerr << "Body shorter than Content-Length" << std::endl;
	}
	
}


const std::string Request::getMethod() 
{ 
	return _method;
}

void		Request::setFd(int	client_fd)
{
	this->_client_fd = client_fd;
}

const std::string	Request::get_uri_location()
{
	return _uri_location.path;
}


void Request::isRequestValid(double client_max_b_size)
{
	int code;
	if (_uri_rooted.size() > 100)
		return (_valid = false, sendError(414));
	if (client_max_b_size != -1 && _body.size() != 0 && _body.size() > client_max_b_size)
		return (_valid = false, sendError(413));
	if ((code = checkMethod(_uri_location.method , _method)) && code != 0)
		return (_valid = false, sendError(code));
	if (_version != "HTTP/1.1")
		return (_valid = false, sendError(505));
	if ((code = checkURI(_uri_rooted)) && code != 0)
		return (_valid = false, sendError(code));

	_valid = true;
}

int Request::checkMethod(std::map<std::string, bool> &methodAvaible, std::string &method) {

	if (methodAvaible.find(method) == methodAvaible.end())
		return (405);
	if (methodAvaible.find(method)->second == false)
		return (405);
	return (0);
}

int Request::checkURI(std::string &filePath) {
	if (!this->get_location_struct().redirect.empty())
		return 0;
	if (filePath[0] == '/')
		filePath = "." + filePath;
	struct stat fileStat;
	if (stat(filePath.c_str(), &fileStat) == 0)
	{
		if (S_ISDIR(fileStat.st_mode))
		{
			if (_uri_location.d_list == true) {
				_file = fileStat;
				return 0;
			}
			if (_uri_location.index.size() > 0)
			{
				for (size_t i = 0; i < _uri_location.index.size(); i++)
				{
					struct stat indexStat;
					std::string indexPath(filePath + _uri_location.index[i]);
					if (stat(indexPath.c_str(), &indexStat) == 0)
					{
						if (S_ISDIR(indexStat.st_mode))
							return (403);
						if (!(indexStat.st_mode & S_IRUSR))
							return (403);
						_file = indexStat;
						return (_uri_rooted =_uri_rooted + _uri_location.index[i], 0);
					}
				}
				return (404);
			}
			return (403);
		}
		if (!(fileStat.st_mode & S_IRUSR))
			return (403);
	}
	else
	{
		return (404);
	}
	_file = fileStat;
	return 0;
}

void Request::sendError(int errorCode) {
	Response response(errorCode, _error_page);
	_sendValue = send(_client_fd, response.get_response().c_str(), response.get_response().size(), 0);
	if (_sendValue > 0)
		std::cout << GREEN << response.get_response() << RESET << std::endl;
}

bool Request::getValid() {
	return _valid;
}

void Request::handle_get(int client_sock, double client_max_b_size) {
	int type = 0;

	if (ends_with(_URI, ".png") || ends_with(_URI, ".jpeg") || ends_with(_URI, ".jpg"))
		type = 1;
	if (type != 1 && client_max_b_size != -1 && _file.st_size > client_max_b_size)
		return (_valid = false, sendError(413));
	std::string path;
	if (_uri_rooted[0] == '.')
		path = _uri_rooted.substr(2, _uri_rooted.size());
	else
		path = _uri_rooted.substr(1, _uri_rooted.size());
	if (type == 1)
		return (_valid = false, sendError(415));
	else {
		std::ifstream file(path.c_str());

		std::string str;
		std::string final_line;
		std::string type;

		if (_uri_location.d_list == true && S_ISDIR(_file.st_mode)) {
			DIR* dir = opendir(path.c_str());
			if (!dir) {
				sendError(403);
				return;
			}
			std::ostringstream html;
			html << "<html><body>";
			struct dirent* entry;
			while ((entry = readdir(dir)) != NULL) {
				std::string name = entry->d_name;
				if (name == "." || name == "..")
					continue;
				html << ". " << name << "<br>";
			}
			html << "</body></html>";
			closedir(dir);
			final_line = html.str();
			type = "html";
		}
		else {
			while (std::getline(file, str)) {
				final_line += str;
			}
			type = path.substr(path.find(".", 0) + 1, path.size());
		}
		std::ostringstream oss;
		oss << final_line.size();
		std::string http_response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/" + type + "\r\n"
			"Content-Length: " + oss.str()  + "\r\n"
			"Connection: close\r\n\r\n" + final_line;
		_sendValue = send(client_sock, http_response.c_str(), http_response.size(), 0);
		if (_sendValue > 0)
			std::cout << GREEN << http_response<< RESET << std::endl;
	}
}

void	Request::handle_delete(int client_sock)
{
	if (!ends_with(_uri_rooted, ".txt") && !ends_with(_uri_rooted, ".jpg"))
	{
		sendError(415);
		return ;
	}
	if (access(_uri_rooted.c_str(), F_OK) != 0)
	{
		sendError(404);
		return ;
	}
	if (remove(_uri_rooted.c_str()) == 0)
	{
		std::string http_response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 27\r\n"
			"Connection: close\r\n\r\n"
			"File deleted successfully.\n";
		_sendValue = send(client_sock, http_response.c_str(), http_response.size(), 0);
		if (_sendValue > 0)
			std::cout << GREEN << http_response << RESET << std::endl;
	}
	else
		sendError(500);
}

void Request::setErrorPage(std::vector<int> error_page) {
	_error_page = error_page;
}

ssize_t Request::getSendValue() {
	return _sendValue;
}

#include "../inc/Response.hpp"

#include <Request.hpp>

#include "../inc/webServ.hpp"

Response::Response()
{}

Response::Response(const std::string &cgi_response)
{
	format_cgi_response(cgi_response);
}

Response::Response(int errorCode, std::vector<int> &error_page) : _errorCode(errorCode)
{
	std::string firstLine;
	if (errorCode == 400)
		firstLine = "HTTP/1.1 400 Bad Request\r\n";
	if (errorCode == 403)
		firstLine = "HTTP/1.1 403 Forbidden\r\n";
	if (errorCode == 404)
		firstLine = "HTTP/1.1 404 Not Found\r\n";
	if (errorCode == 405)
		firstLine = "HTTP/1.1 405 Method Not Allowed \r\n";
	if (errorCode == 413)
		firstLine = "HTTP/1.1 413 Request Entity Too Large \r\n";
	if (errorCode == 414)
		firstLine = "HTTP/1.1 414 Request-URI Too Long \r\n";
	if (errorCode == 415)
		firstLine = "HTTP/1.1 415 Unsupported Media Type \r\n";
	if (errorCode == 500)
		firstLine = "HTTP/1.1 500 Internal Error \r\n";
	if (errorCode == 504)
		firstLine = "HTTP/1.1 504 Gateway Timeout \r\n";
	if (errorCode == 505)
		firstLine = "HTTP/1.1 505 HTTP Version Not Supported \r\n";
	std::ostringstream response;
	std::string error;

	if (std::find(error_page.begin(), error_page.end(), errorCode) == error_page.end())
		error = "<html><body><h1>" + firstLine + "</h1><p>Default Pages.</p></body></html>";
	else {
		std::ostringstream ss;
		ss << errorCode;
		std::string path("error/" + ss.str() + ".html");
		struct stat fileStat;
		if (stat(path.c_str(), &fileStat) != 0)
			error = "<html><body><h1>" + firstLine + "</h1><p>Default Pages.</p></body></html>";
		else if (S_ISDIR(fileStat.st_mode))
			error = "<html><body><h1>" + firstLine + "</h1><p>Default Pages.</p></body></html>";
		else {
			std::ifstream file(path.c_str());
			std::string tempstr;
			while (std::getline(file, tempstr))
				error += tempstr;
		}
	}

	response << firstLine;
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << error.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << error;
	_response = response.str();
}

Response::~Response()
{}

const std::string &Response::get_response()
{
	return this->_response;
}
void Response::format_cgi_response(std::string cgi_response)
{
	size_t header_end = cgi_response.find("\r\n\r\n");
	if (header_end == std::string::npos)
	{
		std::cerr << "Malformed CGI response: missing header-body separator. Setting default Content-Type text/plain and adding content length" << std::endl;
		_body = cgi_response;
		_headers["Content-Type"] = "text/plain";
		_headers["Content-Length"] = intToString(_body.size());
	}
	else
	{
		std::string header_str = cgi_response.substr(0, header_end);
		_body = cgi_response.substr(header_end + 4);  // skip \r\n\r\n

		std::istringstream header_stream(header_str);
		std::string line;
		while (std::getline(header_stream, line))
		{
			// Remove trailing \r (from lines ending in \r\n)
			if (!line.empty() && line[line.length() - 1] == '\r')
				line.erase(line.length() - 1);


			size_t pos = line.find(": ");
			if (pos == std::string::npos)
			{
				std::cerr << "Warning: Malformed CGI header - " << line << std::endl;
				continue;
			}
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 2);
			
			_headers[key] = value;
		}
	}

	std::string formatted_response = "HTTP/1.1 200 OK\r\n";
	if (_headers.find("Content-Type") == _headers.end())
		_headers["Content-Type"] = "text/html";
	if (_headers.find("Content-Length") == _headers.end())
		_headers["Content-Length"] = intToString(_body.size());

	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
		formatted_response += it->first + ": " + it->second + "\r\n";

	formatted_response += "\r\n" + _body;
	this->_response = formatted_response;
}

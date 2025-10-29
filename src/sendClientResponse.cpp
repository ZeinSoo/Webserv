#include "../inc/webServ.hpp"
#include "../inc/Request.hpp"

void Webserv::sendClientResponse(int client_sock) {

	std::string requestReceived = _clientRequest[client_sock];

	Request request(requestReceived);
	request.setFd(client_sock);
	// Parse and handle
	request.parseRequest();
	request.set_abs_root_path(_commSockets[client_sock]->absolute_root);
	if (request.parse_uri_path((_commSockets[client_sock])->locations) < 0) {
		request.sendError(404);
		if (request.getSendValue() <= 0) {
			close(client_sock);
			return ;
		}
	}
	
	request.setErrorPage(_commSockets[client_sock]->error_page);
	request.isRequestValid(_commSockets[client_sock]->client_max_b_size);

	// Prepare for writing response
	if (request.getValid() == true && request.getSendValue() <= 0) {
		close(client_sock);
		return ;
	}

	//REDIRECT
	if (!request.get_location_struct().redirect.empty())
	{
		std::string body = "<html><body><p>Redirecting to <a href=\"" + request.get_location_struct().redirect + "\">" + request.get_location_struct().redirect + "</a>...</p></body></html>";
		std::ostringstream oss;
		oss << body.size();	
		std::string response =
			"HTTP/1.1 301 Moved Permanently\r\n"
			"Location: " + request.get_location_struct().redirect + "\r\n"
			"Content-Type: text/html; charset=UTF-8\r\n"
			"Content-Length: " + oss.str() + "\r\n"
			"Connection: close\r\n\r\n" + body;
		if (send(client_sock, response.c_str(), response.length(), 0) <= 0) {
			close(client_sock);
			return;
		}
		return;
	}
	
	//CGI
	if (request.getValid() == true && (request.is_CGI() && request.getMethod() != "DELETE"))
		request.handle_CGI();
	//METHODS
	else if (request.getValid() == true && request.getMethod() == "GET")
		request.handle_get(client_sock, _commSockets[client_sock]->client_max_b_size);
	else if (request.getValid() == true && request.getMethod() == "DELETE")
		request.handle_delete(client_sock);
	else if (request.getValid() == true && request.getMethod() == "POST") {
		request.sendError(403);
	}
	if (request.getSendValue() <= 0) {
		close(client_sock);
		return ;
	}

	// // Switch back to EPOLLIN for next request
	// event.events = EPOLLIN;
	// if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client_sock, &event) < 0)
	// {
	// 	std::ostringstream errstr;
	// 	errstr << "epoll_ctl (EPOLLIN restore) failed for socket " << client_sock;
	// 	throw std::runtime_error(errstr.str());
	// }
	close(client_sock);
	_clientRequest.erase(client_sock);
}
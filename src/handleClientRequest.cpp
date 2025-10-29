#include "../inc/webServ.hpp"
#include "../inc/Request.hpp"

#define BUFFER_SIZE 4096

void Webserv::handleClientRequest(int client_sock)
{
	std::string requestReceived;
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read;

	// Read headers first
	memset(buffer, '\0', BUFFER_SIZE);
	usleep(1000);
	bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
	if (bytes_read == 0 || bytes_read == -1)
	{
		close(client_sock);
		return;
	}
	requestReceived.append(buffer, bytes_read);
	size_t header_end = requestReceived.find("\r\n\r\n");
	if (header_end != std::string::npos) {
	    // Headers are fully read
	    //size_t content_length = 0;
	    std::string header_part = requestReceived.substr(0, header_end);
		std::istringstream iss(header_part);
	    std::string line;
	    /* while (std::getline(iss, line))
		{
	        if (line.find("Content-Length:") != std::string::npos)
			{
	            char *endptr;
				content_length = std::strtol(line.substr(16).c_str(), &endptr, 10);
	            break;
	        }
	    } */
	}
	if (requestReceived.empty())
		return;
	std::cout << "\nNew Request Received :\n" << ORANGE << requestReceived << RESET << std::endl;
	struct epoll_event event;
	event.data.fd = client_sock;
	event.events = EPOLLOUT;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client_sock, &event) < 0) {
		std::ostringstream errstr;
		errstr << "epoll_ctl (EPOLLOUT) failed for socket " << client_sock;
		throw std::runtime_error(errstr.str());
	}
	_clientRequest[client_sock] = requestReceived;
}

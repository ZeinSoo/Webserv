#include "../inc/webServ.hpp"
#include "../inc/Request.hpp"

volatile bool stop = false;

Webserv::Webserv() {}

Webserv::~Webserv()
{
	std::map<int, int>::iterator it;
	for (size_t i = 0; i < _servers.size(); i++)
	{
		for (it = _servers[i].listeningSockets.begin(); it != _servers[i].listeningSockets.end(); ++it)
			close(it->second);
	}
	close(_epoll_fd);
}

std::string intToString(int num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}

void Webserv::setupServers()
{
	int sock, port;
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd < 0)
		throw std::runtime_error("epoll_create1 failed");

	for (size_t i = 0; i < _servers.size(); ++i) {
		for (size_t j = 0; j < _servers[i].ports.size(); ++j) {
			port = _servers[i].ports[j];
			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				throw std::runtime_error("socket() failed");

			if (!_listeningSockets.count(port))
				_listeningSockets[sock] = &_servers[i];
			int opt = 1;
			if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
				throw std::runtime_error("setsockopt() failed");

			struct addrinfo hints, *res;
			std::memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_PASSIVE;

			std::string port_str = intToString(port);
			if (getaddrinfo(_servers[i].server_name.c_str(), port_str.c_str(), &hints, &res) != 0)
				throw std::runtime_error("getaddrinfo() failed");

			if (bind(sock, res->ai_addr, res->ai_addrlen) < 0) {
				close(sock);
				freeaddrinfo(res);
				throw std::runtime_error("bind() failed");
			}
			freeaddrinfo(res);

			if (listen(sock, SOMAXCONN) < 0) {
				close(sock);
				throw std::runtime_error("listen() failed");
			}

			fcntl(sock, F_SETFL, O_NONBLOCK);
			_servers[i].listeningSockets[port] = sock;
			struct epoll_event event;
			event.events = EPOLLIN;
			event.data.fd = sock;

			if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, sock, &event) < 0) {
				close(sock);
				throw std::runtime_error("epoll_ctl() failed to add listening socket");
			}
			std::cout << "Listening on port " << port << " (socket " << sock << ")" << std::endl;
		}
		printServerInfo(_servers[i]);
	}
}

void handle_signal(int signal)
{
	if (signal == SIGINT) {
		std::cout << "\nClosing the server..." << std::endl;
		stop = true;
	}
}

void Webserv::run()
{
	if (this->_servers.empty())
	{
		std::cerr << "No servers valid, exiting." << std::endl;
		return ;
	}
	std::signal(SIGINT, handle_signal);
	const int MAX_EVENTS = 1024;
	struct epoll_event events[MAX_EVENTS];

	std::cout << "Web server running..." << std::endl;
	while (stop == false) {
		int n_events = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
		if (n_events < 0)
			throw std::runtime_error("epoll_wait() failed");
		for (int i = 0; i < n_events; ++i) 
		{
			int fd = events[i].data.fd;
			if (_listeningSockets.count(fd))
				handleNewConnection(fd);
			else if (events[i].events & EPOLLIN)
				handleClientRequest(fd);
			else if (events[i].events & EPOLLOUT)
				sendClientResponse(fd);
		}
	}
}

void Webserv::handleNewConnection(int listening_sock)
{
	struct sockaddr_in clientAddr;
	socklen_t clientlen = sizeof(clientAddr);
	int comm_sock = accept(listening_sock, (struct sockaddr*)&clientAddr, &clientlen);

	if (comm_sock < 0) {
		std::cerr << "Failed to accept new connection on socket " << listening_sock << std::endl;
		return;
	}

	fcntl(comm_sock, F_SETFL, O_NONBLOCK);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = comm_sock;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, comm_sock, &event) < 0) {
		std::cerr << "epoll_ctl() failed while adding comm socket " << comm_sock << std::endl;
		close(comm_sock);
		return;
	}
	_commSockets[comm_sock] = _listeningSockets[listening_sock]; // Map comm socket to server

}

void Webserv::printServerInfo(t_serverConfig &server)
{
	std::cout << "[INFO] Server Name : " <<  server.server_name << std::endl;
	std::cout  << "[INFO] Port(s) : ";
	for (size_t i = 0; i < server.ports.size(); i++)
		std::cout << "" << server.ports[i] << " ";
	std::cout << "\n" << std::endl;
}
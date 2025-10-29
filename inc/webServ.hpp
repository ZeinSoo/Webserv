#pragma once

#include <map>
#include <vector>
#include <stack>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/timerfd.h>
#include <iostream>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <netdb.h>
#include <sstream>
#include <string>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <limits.h>
#include <unistd.h>
#include <bits/stdc++.h>

#define RESET "\x1b[0m"
#define ORANGE "\x1b[38;5;216m"
#define GREEN "\x1b[38;5;84m"

typedef struct s_routing_info
{
	std::string path;
	std::string absolute_path;
	std::string extension;
	std::string root;
	double client_max_b_size;
	std::string upload_dir;
	std::string cgi;
	std::string redirect;
	bool d_list;
	std::vector<std::string> index;
	std::map<std::string, bool> method;
}	t_routing_info;

typedef struct s_serverConfig
{
	std::vector<int> ports;
	std::string server_name;
	std::string host;
	std::vector<int> error_page;
	double client_max_b_size;
	std::vector<std::string> index;
	std::vector<t_routing_info> locations;
	std::string root;
	std::string absolute_root;
	std::map<int, int> listeningSockets;
} t_serverConfig;

class Webserv
{
	private:
		int							_epoll_fd;
		std::vector<t_serverConfig> _servers;
		std::map<int, t_serverConfig*> _listeningSockets;
		std::map<int, t_serverConfig*> _commSockets;
		std::map<int, std::string> _clientRequest;

	public:
		Webserv();
		~Webserv();

		void	parseConf(const std::string &conf);
		void	setupServers();
		void	run();
		void	handleNewConnection(int listeningSocket);
		void	handleClientRequest(int  client_sock);
		void	sendClientResponse(int  client_sock);
		void	printServerInfo(t_serverConfig &server);

};

std::string intToString(int num);
std::string get_absolute_path(std::string &relative_path);
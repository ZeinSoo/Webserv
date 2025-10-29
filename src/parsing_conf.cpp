#include "../inc/webServ.hpp"
#include "../inc/Request.hpp"

int checkPorts(std::vector< t_serverConfig >servers, t_serverConfig data)
{
	std::vector< int > s_ports;
	for (int i = 0; i < (int)servers.size(); i++)
	{
		for (int j = 0; j < (int)servers[i].ports.size(); j++)
		{
			s_ports.push_back(servers[i].ports[j]);
		}
	}
	for (int i = 0; i < (int)data.ports.size(); i++)
	{
		for (int j = 0; j < (int)s_ports.size(); j++)
		{
			if (s_ports[j] == data.ports[i])
				return 1;
		}
	}
	for (int i = 0; i < (int)data.ports.size(); i++)
	{
		for (int j = 0; j < (int)data.ports.size(); j++)
		{
			if (j != i && data.ports[j] == data.ports[i])
				return 1;
		}
	}
	return 0;
}

void	Webserv::parseConf(const std::string &conf)
{
	std::string line;
	std::string token;
	std::vector<std::string> directives;
	std::ifstream file(conf.c_str());
	int i = 0;
	while (std::getline(file, line))
	{
		i = 0;
		while (line[i])
		{
			if (line[i] == ' ')
			{
				if (!token.empty())
					directives.push_back(token);
				i++;
				while (!token.empty())
					token.erase();
				continue;
			}
			if (line[i] == '{')
			{
				if (!token.empty())
					directives.push_back(token);
				i++;
				while (!token.empty())
					token.erase();
				directives.push_back("{");
				continue;
			}
			if (line[i] == '}')
			{
				if (!token.empty())
					directives.push_back(token);
				i++;
				while (!token.empty())
					token.erase();
				directives.push_back("}");
				continue;
			}
			if (line[i] == ';')
			{
				if (!token.empty())
					directives.push_back(token);
				i++;
				while (!token.empty())
					token.erase();
				directives.push_back(";");
				continue;
			}

			if (line[i] >= 33 && line[i] <= 126)
			{
				token += line[i];
			}
			i++;
		}
	}
	std::vector<t_serverConfig> servers;
	for (int i = 0; i < (int)directives.size(); i++)
	{
		if (directives[i] == "server" && directives[i + 1] == "{")
		{
			bool end_server = false;
			t_serverConfig data;
			t_routing_info nlocation;
			data.server_name = "";
			data.host = "";
			data.client_max_b_size = -1;
			nlocation.upload_dir = "";
			nlocation.method.insert(std::pair<std::string, float>("GET", false));
			nlocation.method.insert(std::pair<std::string, float>("POST", false));
			nlocation.method.insert(std::pair<std::string, float>("DELETE", false));
			nlocation.path = "";
			nlocation.d_list = false;
			nlocation.redirect = "";
			nlocation.extension = "";
			nlocation.root = "";
			bool loc = false;
			i += 2;
			while (end_server == false && i < (int)directives.size())
			{
				if (directives[i] == "listen" && loc == false && end_server == false) // !loc
				{
					i++;
					while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
					{
						data.ports.push_back(atoi(directives[i].c_str()));
						i++;
					}
				}
				if (directives[i] == "server_name" && loc == false && end_server == false) // !loc
				{
					i++;
					while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
					{
						data.server_name = directives[i];
						i++;
					}
				}
				if (directives[i] == "host" && loc == false && end_server == false) // !loc
				{
					i++;
					while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
					{
						data.host = directives[i];
						i++;
					}
				}
				if (directives[i] == "error_page" && loc == false && end_server == false) // !loc
				{
					i++;
					while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
					{
						data.error_page.push_back(atoi(directives[i].c_str()));
						i++;
					}
				}
				if (directives[i] == "client_max_body_size" && end_server == false) // loc && !loc
				{
					i++;
					if (loc)
					{
						while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
						{
							nlocation.client_max_b_size = atof(directives[i].c_str());
							i++;
						}
					}
					else
					{
						while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
						{
							data.client_max_b_size = atof(directives[i].c_str());
							i++;
						}
					}
				}
				if (directives[i] == "root" && end_server == false) // loc et !loc
				{
					i++;
					if (loc)
					{
						while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
						{
							nlocation.root = directives[i];
							i++;
						}
					}
					else
					{
						while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
						{
							data.root = directives[i];
							i++;
						}
					}
				}
				if (directives[i] == "directory_listing" && loc && end_server == false) // loc
				{
					i++;
					while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
					{
						if (directives[i] == "on")
							nlocation.d_list = true;
						i++;
					}
				}
				if (directives[i] == "index" && end_server == false) // loc et !loc
				{
					i++;
					if (loc)
					{
						while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
						{
							nlocation.index.push_back(directives[i]);
							i++;
						}
					}
					else
					{
						while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
						{
							data.index.push_back(directives[i]);
							i++;
						}
					}
				}
				if (directives[i] == "cgi_pass" && loc && end_server == false) // loc
				{
					i++;
					while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
					{
						nlocation.cgi = directives[i];
						i++;
					}
				}
				if (directives[i] == "allow_methods" && loc && end_server == false) // loc
				{
					i++;
					while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
					{
						if (directives[i] == "GET")
							nlocation.method["GET"] = true;
						else if (directives[i] == "POST")
							nlocation.method["POST"] = true;
						else if (directives[i] == "DELETE")
							nlocation.method["DELETE"] = true;
						i++;
					}
				}
				if (directives[i] == "upload_dir" && loc && end_server == false) // loc
				{
					i++;
					while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
					{
						nlocation.upload_dir = directives[i];
						i++;
					}
				}
				if (directives[i] == "redirect" && loc && end_server == false) // loc
				{
					i++;
					while (directives[i] != ";" && directives[i] != "}" && i < (int)directives.size())
					{
						nlocation.redirect = directives[i];
						i++;
					}
				}
				if (directives[i] == "location" && loc == false && end_server == false) // !loc
				{
					i++;
					nlocation.path = directives[i];
					i++;
					if (directives[i] == "{")
						loc = true;
				}
				if (directives[i] == "}")
				{
					if (loc)
					{
						loc = false;
						data.locations.push_back(nlocation);
						nlocation.path = "";
						nlocation.root = "";
						nlocation.d_list = false;
						nlocation.redirect = "";
						nlocation.extension = "";
						nlocation.method.clear();
						nlocation.cgi.clear();
						nlocation.index.clear();
					}
					else
						end_server = true;
				}
				if (end_server == false)
					i++;
			}
			if (end_server == true)
			{
				if (data.root != "" && !data.ports.empty() && data.server_name != "" && !checkPorts(servers, data))
					servers.push_back(data);
				else
					std::cerr << "server number " << servers.size() << " is not valid" << std::endl;
			}
			else if (end_server == false && (directives.begin() + i) == directives.end())
			{
				std::cerr << "server number " << servers.size() << " has encountered an error" << std::endl;
				break ;
			}
		}
	}
	if (!servers.empty())
	{
		for (int i = 0; i < (int)servers.size(); i++)
		{
			for (int j = 0; j < (int)servers[i].locations.size(); j++)
			{
				std::size_t pos = std::string::npos;
				servers[i].locations[j].absolute_path = servers[i].root + servers[i].locations[j].path;

				pos = servers[i].locations[j].absolute_path.find("./");
				while (pos != std::string::npos)
				{
					servers[i].locations[j].absolute_path.erase(pos, 1);
					pos = servers[i].locations[j].absolute_path.find("./");
				}
				pos = servers[i].locations[j].absolute_path.find("//");
				while (pos != std::string::npos)
				{
					servers[i].locations[j].absolute_path.erase(pos, 1);
					pos = servers[i].locations[j].absolute_path.find("//");
				}
			}
		}
		this->_servers = servers;
		return ;
	}
	this->_servers = std::vector<t_serverConfig>(0);
}
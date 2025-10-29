#include "../inc/webServ.hpp"

int	main(int argc, char **argv)
{
	std::string config_path;

	if (argc == 1)
		config_path = "default.conf";
	else if (argc != 2)
		return 1;
	else
		config_path = argv[1];
	try
	{
		Webserv webserv = Webserv();
		webserv.parseConf(config_path);
		webserv.setupServers();
		webserv.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return 0;
}

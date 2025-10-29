#include "../inc/webServ.hpp"
#include "../inc/Request.hpp"
#include "../inc/Response.hpp"

int ends_with(const std::string &str, const std::string &suffix)
{
	return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int	Request::is_CGI()
{
	return (ends_with(_URI, ".py") || ends_with(_URI, ".sh") || ends_with(_URI, ".js") || ends_with(_URI, ".php")); //rajouter les extensions gérées
}

std::string get_absolute_path(std::string &relative_path)
{
	char cwd[PATH_MAX];
	if (!getcwd(cwd, sizeof(cwd)))
		throw std::runtime_error("Failed to get CWD");
	if (relative_path[0] == '.')
	{
		relative_path = relative_path.substr(1);
	}
	return std::string(cwd) + relative_path;
}

void Request::handle_CGI()
{
	int inPipe[2], outPipe[2];
	if (pipe(inPipe) == -1 || pipe(outPipe) == -1)
		throw std::runtime_error("Pipe failed");

	_path = get_absolute_path(_uri_rooted);

	if (access(_path.c_str(), F_OK) == -1)
		throw std::runtime_error("File does not exist: " + _path);
	if (access(_path.c_str(), X_OK) == -1)
		throw std::runtime_error("Permission denied: " + _path);

	pid_t pid = fork();
	if (pid < 0)
		throw std::runtime_error("Fork failed");

	if (pid == 0) {
		// Child
		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);
		close(inPipe[1]); close(outPipe[0]);

		int devnull = open("/dev/null", O_WRONLY);
		if (devnull != -1) {
			dup2(devnull, STDERR_FILENO);
			close(devnull);
		}

		setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
		setenv("REDIRECT_STATUS", "1", 1);
		setenv("REQUEST_METHOD", _method.c_str(), 1);
		setenv("SCRIPT_FILENAME", _path.c_str(), 1);
		setenv("SCRIPT_NAME", _URI.c_str(), 1);
		setenv("PATH_INFO", _URI.c_str(), 1);
		setenv("REQUEST_URI", _URI.c_str(), 1);
		setenv("ROOT_PATH", get_absolute_path(this->_URI).c_str(), 1);
		setenv("UPLOAD_DIR", get_absolute_path(this->_uri_location.upload_dir).c_str(), 1);
		setenv("RELATIVE_UPLOAD_DIR", this->_uri_location.upload_dir.c_str(), 1);
		
		if (_method == "POST")
		{
			setenv("CONTENT_LENGTH", _headers["Content-Length"].c_str(), 1);
			setenv("CONTENT_TYPE", _headers["Content-Type"].c_str(), 1);
		}
		setenv("QUERY_STRING", _query_string.c_str(), 1);
		char *args[2];
		if (!this->_uri_location.cgi.empty())
		{
			args[0] = (char *)_uri_location.cgi.c_str();
			args[1] = NULL;
		}
		else if (ends_with(_URI, ".php"))
		{
			args[0] = (char *)"/usr/bin/php-cgi";
			args[1] = NULL;
		}
		else if (ends_with(_URI, ".py"))
		{
			args[0] = (char *)"/usr/bin/python3";
			args[1] = NULL;
		}
		else 
		{
			args[0] = (char *)_path.c_str();
			args[1] = NULL;
		}
		execve(args[0], args, environ);
		perror("execve failed");
		_exit(1);
	}
	else 
	{
		// Parent
		close(inPipe[0]); close(outPipe[1]);

		if (_method == "POST")
		{
			ssize_t written = write(inPipe[1], _body.c_str(), _body.size());
			if (written < 0)
			{
				return ;
			}
		}
	}
		close(inPipe[1]);

		int flags = fcntl(outPipe[0], F_GETFL, 0);
		fcntl(outPipe[0], F_SETFL, flags | O_NONBLOCK);

		std::string cgi_output;
		char buffer[1024];
		ssize_t n;

		int status;
		pid_t result;
		int elapsed = 0;
		const int timeout = 5;

		while (elapsed < timeout)
		{
			while ((n = read(outPipe[0], buffer, sizeof(buffer) - 1)) > 0)
			{
				buffer[n] = '\0';
				cgi_output += buffer;
			}
			result = waitpid(pid, &status, WNOHANG);
			if (result != 0)
				break;
			sleep(1);
			++elapsed;
		}
		if (WIFEXITED(status))
		{
			int exit_code = WEXITSTATUS(status);
			if (exit_code != 0)
			{
				sendError(500);
				return ;
			}
		}
	close(outPipe[0]);
	if (result == 0) {
		std::cerr << "CGI execution timeout." << std::endl;
		kill(pid, SIGKILL);
		waitpid(pid, &status, 0);
		sendError(504);
		return ;
	}
	if (cgi_output.find("Status: 415") != std::string::npos) {
		sendError(415);
		return ;
	}
	if (cgi_output.find("Status: 400") != std::string::npos) {
		sendError(400);
		return ;
	}
	if (cgi_output.find("Status: 403") != std::string::npos) {
		sendError(403);
		return ;
	}
	if (cgi_output.find("\r\n\r\n") == std::string::npos)
	{
		sendError(500);
		return ;
	}
	Response res(cgi_output);
	const std::string &response = res.get_response();
	_sendValue = send(_client_fd, response.c_str(), response.size(), 0);
	if (_sendValue <= 0)
		return ;
	else
		std::cout << GREEN << response << RESET << std::endl;
}

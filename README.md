# Webserv

HTTP server implemented in C++98: multi-port management with epoll, Nginx-style configuration, CGI support (Python/PHP), file upload, and custom error pages.

## Prerequisites
- C++ compiler (g++/clang++)
- Make
- PHP and Python (for CGI scripts)

## Quick Start
1) Compile the project
```bash
make
```

2) Start the server (default configuration)
```bash
./webserv
```
Or with a custom configuration file:
```bash
./webserv config.conf
```

3) Access the server
- URL: http://localhost:8080
- Alternative URL: http://localhost:8090

## Available Make Commands
- `make` or `make all`: Compile the project
- `make clean`: Remove object files
- `make fclean`: Remove object files and executable
- `make re`: Fully recompile the project

## Architecture and Features
The web server implements the following features:
- **Virtual multi-servers**: multiple server configurations with distinct ports and server_name
- **HTTP request handling**: GET, POST, DELETE
- **CGI**: Python and PHP script execution
- **File upload**: upload to configurable directories
- **Directory listing**: list files in a directory
- **Redirections**: configurable HTTP redirections
- **Custom error pages**: 400, 403, 404, 405, 413, 414, 415, 500, 504, 505
- **Advanced configuration**: request size limits, allowed methods per location

## Configuration Structure
The configuration file (`default.conf`) allows you to define:
- `server_name`: virtual server name
- `listen`: listening port(s)
- `root`: static files root directory
- `client_max_body_size`: maximum request body size
- `error_page`: custom error pages
- `location`: routes with specific configuration
  - `root`: root directory for this location
  - `index`: default index files
  - `allow_methods`: allowed HTTP methods
  - `directory_listing`: enable directory listing
  - `upload_dir`: destination directory for uploads
  - `cgi_pass`: CGI script to execute
  - `redirect`: redirection URL

## Entry Points (default configuration)
- Home page: `http://localhost:8080/`
- File upload: `http://localhost:8080/root/cgi-bin/`
- Uploaded files list: `http://localhost:8080/root/cgi-bin/uploads/`
- Google redirect: `http://localhost:8080/google/` (redirects to google.com)
- Secondary server: `http://127.0.0.2:9090/`

## Configuration Examples
### Simple server
```
server {
    server_name localhost;
    listen 8080;
    root ./root/;
    
    location / {
        index index.html;
        allow_methods GET POST;
    }
}
```

### Location with CGI
```
location /cgi-bin/ {
    allow_methods GET POST;
    cgi_pass cgi-bin/script.py;
}
```

### Location with upload
```
location /upload/ {
    allow_methods GET POST DELETE;
    upload_dir /root/uploads/;
    directory_listing on;
}
```

## Test Files
- `test.conf`: alternative test configuration
- `siege.conf`: configuration for load testing with Siege

## Technical Notes
- C++98 standard
- epoll for I/O event management
- Non-blocking architecture
- Nginx-inspired configuration parsing
- Full HTTP/1.1 request support

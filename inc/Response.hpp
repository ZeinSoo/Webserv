#ifndef RESPONSE_HPP
#define RESPONSE_HPP

# include "webServ.hpp"


class Response
{
    private:
        std::map<std::string, std::string> _headers;
        std::string _body;
        std::string _response;
		int _errorCode;

    public:
        Response();
		Response(const std::string &cgi_response);
		Response(int errorCode, std::vector<int> &error_page);
        ~Response();

		const std::string &get_response();
        void	format_cgi_response(std::string cgi_response);
		void	format_get_response(std::string body, int type);

};



#endif //RESPONSE_HPP

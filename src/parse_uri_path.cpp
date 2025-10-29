#include "../inc/webServ.hpp"
#include "../inc/Request.hpp"

std::string root_loc_uri(t_routing_info res, std::string uri)
{
	if (uri.size() < res.path.size())
		return (res.root + uri.substr(res.path.size() - 1));
	return (res.root + uri.substr(res.path.size()));
}


std::vector<std::string> tokenise_path(std::string uri)
{
	std::vector<std::string> result;
	std::string elem = "";

	int i = 0;
	while (i < (int)uri.size())
	{
		if (uri[i] == '/' || (uri.begin() + i == uri.end()))
		{
			if (elem != "")
			{
				result.push_back(elem);
				elem = "";
			}
		}
		else
			elem += uri[i];
		i++;
	}
	if (uri.begin() + i == uri.end() && elem != "")
	{
		result.push_back(elem);
		elem = "";
	}
	return result;
}

int is_valid_path(std::vector<std::string> tok, std::vector<std::string> nloc_path)
{
	for (int i = 0; i < (int)nloc_path.size(); i++)
	{
		if (i < (int)tok.size())
		{
			if (tok[i] != nloc_path[i])
				return 0;
		}
		else
			return 0;
	}
	return 1;
}

int		Request::parse_uri_path(std::vector<t_routing_info> locations)
{
	t_routing_info res;
	int m_res = -1;
	std::string uri = this->_URI;
	std::vector<std::string> tok = tokenise_path(uri);
	bool found_extension = false;
	if (tok.size() == 0)
	{
		for (int i = 0; i < (int)locations.size(); i++)
		{
			if (locations[i].path == "/" || locations[i].path == "./")
			{
				res = locations[i];
				_uri_location = res;
				_uri_rooted = uri;
				if (res.root != "")
					_uri_rooted = root_loc_uri(res, uri);
				return 0;
			}
		}
		return 0;
	}
	for (int i = 0; i < (int)locations.size(); i++)
	{
		int check = 0;
		std::vector<std::string> nloc_path = tokenise_path(locations[i].path);
		if (nloc_path.size() > 0 && nloc_path[0] == ".")
			nloc_path.erase(nloc_path.begin());
		std::size_t pos = std::string::npos;
		if (!is_valid_path(tok, nloc_path))
			continue;
		if (locations[i].extension != "")
		{
			if (tok.back().find(locations[i].extension) == std::string::npos)
				continue;
		}
		if (locations[i].extension != "")
			pos = tok.back().find(locations[i].extension);
		if (pos != std::string::npos && !found_extension)
		{
			m_res = i;
			found_extension = true;
		}
		else if (pos == std::string::npos && found_extension)
		{
			continue;
		}
		else
		{
			if (nloc_path.size() == 0 && m_res == -1)
				m_res = i;
			for (int j = 0; j < (int)nloc_path.size(); j++)
			{
				if (j < (int)tok.size())
				{
					if (tok[j] == nloc_path[j])
						check++;
				}
				if (check != 0 && j == (int)tok.size() - 1)
				{
					res = locations[i];
					_uri_location = res;
					_uri_rooted = uri;
					if (res.root != "")
						_uri_rooted = root_loc_uri(res, uri);
					return i;
				}
				if (check != 0 && j == (int)nloc_path.size() - 1)
				{
					m_res = i;
				}
			}
		}
	}
	if (m_res != -1)
	{
		res = locations[m_res];
		_uri_location = res;
		_uri_rooted = uri;
		if (res.root != "")
			_uri_rooted = root_loc_uri(res, uri);
		return 0;
	}
	return -1;
}
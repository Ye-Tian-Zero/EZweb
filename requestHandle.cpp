#include "requestHandle.h"
#include <algorithm>
#include <sys/stat.h>
#include <sstream>

using std::find;
using std::stringstream;

void 
requestHandle::dispart(string &command, string& content, const string& text)
{
	command.clear();
	content.clear();
	
/*	
 *	if(text.find("png", 0 ) != -1)
 *	{
 *		EZ_WARN(text);
 *	}
 */

	auto iter = find(text.begin(), text.end(), ' ');
	if(iter == text.end())
		return;

	command.assign(text.begin(), iter);
	content.assign(iter + 1, text.end());

	if(command.back() == ':')
	{
		command.pop_back();
	}
}

void
requestHandle::processCmd(const string& command, const string& content, int connect_fd, string peer)
{
	if(connect_fd == INT_MIN)
	{
		connect_fd = _connect_fd;
	}

	if(peer.empty())
	{
		peer = EZ_peer;
	}

	if(command == "GET")
	{
		EZ_INFO(peer + " -> " + command + " " + content);

		string dir = extractFileDir(content);

		close(file_fd);

		file_fd = open(dir.c_str(), O_RDONLY);

		if(file_fd >= 0)
		{
			string header = "HTTP/1.1 200 ok\r\n";
			struct stat file_info;
			fstat(file_fd, &file_info);
			string content_length = "Content-Length: "; //asdkljklasdjkgljakldsgjk
			stringstream strm;
			strm << file_info.st_size;
			content_length += strm.str();

			cout << "write start at " << connect_fd << endl;
			Writen(connect_fd, header.c_str(), header.size());
			Writen(connect_fd, content_length.c_str(), content_length.size());
			Writen(connect_fd, "\r\n\r\n", 4);

			char buffer[4096];
			int cnt(0);
			do{

				cnt = read(file_fd, buffer, 4096);
				Writen(connect_fd, buffer, cnt);

			}while(cnt);

			cout << "endl" << endl;
		}
		else
		{
			EZ_WARN("Can't find " + dir + "\n");
			string content_length = "Content-Length: 0\r\n";
			string header = "HTTP/1.1 404 NotFound\r\n";
			Writen(connect_fd, header.c_str(), header.size());
			Writen(connect_fd, content_length.c_str(), content_length.size());
			Writen(connect_fd, "\r\n", 2);

		}
	}

	return;
}

string
requestHandle::extractFileDir(const string& content)
{
	stringstream strm(content);
	string path;

	strm >> path;

	int pos = path.find_first_of('?');

	if(pos != string::npos)
	{
		path.erase(path.begin() + pos, path.end());

		/*
		 * Here we ignore the data in "get" method
		 */
	}

	return path == "/"? string(root + '/' + index) : string(root + path);
}


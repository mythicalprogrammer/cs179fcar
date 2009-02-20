#include "strparse.h"

strparse::strparse()
{
	passed_string = "";
}

strparse::strparse( string s )
{
	passed_string = s;
}

void strparse::add_string( string s )
{
	passed_string = s;
}

string strparse::get_next_token(string s)
{
	if( passed_string == "" )
		return "";

	unsigned len = strcspn( passed_string.c_str(), s.c_str() );
	string ret;
	if( len != passed_string.length() )
	{
		ret = passed_string.substr(0, len);
		passed_string = passed_string.substr(len +1);
	}
	else
	{
		ret = passed_string;	
		passed_string = "";
	}
	
	return ret;
}

string strparse::get_str_between(string s, string e)
{
	string ret;
	size_t start = passed_string.find(s);
	size_t end;

	if( start == string::npos )
		return "";
	
	end = passed_string.find(e,start + s.length());
	if( end == string::npos )
	{
		ret = passed_string.substr(int(start) + s.length() );
		passed_string = "";
	}
	else
	{
		ret = passed_string.substr(int(start) + s.length(), int(end) - ( int(start) + s.length() ) );
		passed_string = passed_string.substr( int(end) + e.length() );
	}
	return ret;
}

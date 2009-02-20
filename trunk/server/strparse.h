#ifndef __STRPARSE_H__
#define __STRPARSE_H__

#include <string>

using namespace std;

class strparse
{
public:
	strparse();
	strparse(string);

	void add_string(string);
	
	string get_next_token(string); 
	string get_str_between(string,string);

private:
string passed_string;
int pos;
};
#endif

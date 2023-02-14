#pragma once
#include <fstream>
#include <queue>
#include <tuple>
#include <map>
#include "WAVfileConverter.h"


class parser
{
public:
	//basically, the converter and the tuple: start timecode and end timecode
	std::queue <std::tuple<WAVfileConverter*, int, int>> modifiers_queue;
	void parse(std::string input_path, std::vector<FILE*>*, std::vector<Header>*);
};

inline std::vector<std::string> split(std::string input_str, char separator)
{
	std::vector<std::string> result;
	std::string token;
	int found_index = 0;
	while (found_index != -1)
	{
		found_index = input_str.find(separator);
		if (found_index == -1)
			token = input_str;
		else
			token = input_str.substr(0, found_index);
		result.push_back(token);
		input_str.erase(0, found_index + 1);
	}

	return result;
}
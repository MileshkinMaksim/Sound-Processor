#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <Windows.h>

class Help
{
public:
	typedef std::string(__stdcall* h_func)();

	std::vector <HINSTANCE> mod_handles;
	std::string using_mute_help;
	std::string using_mix_help;
	std::string using_reverse_help;

	std::string functionality_mute_help;
	std::string functionality_mix_help;
	std::string functionality_reverse_help;

	std::string import_functions_help;

	Help(std::string);

	void get_functions_config_file_help();
	void get_launching_program_help();
	void get_coding_extesions_help();
	void get_config_file_set_help();
};


#include "Help.h"

Help::Help(std::string path_to_mods) 
{
	this->using_mute_help = "To use mute type: mute <start timecode> <end timecode>";
	this->using_mix_help = "To use mix type: mix $<second stream id> <start timecode> <end timecode> / mix $<second stream id> <start timecode>. \nWith the second way, converter will be applied for the whole track";
	this->using_reverse_help = "To use reverse type: reverse <start timecode> <end timecode>";

	this->functionality_mute_help = "Mutes track from start timecode (in seconds) to end timecode (in seconds)";
	this->functionality_mix_help = "Mixes main track with second stream (id given) from start timecode (in seconds) to end timecode (in seconds)";
	this->functionality_reverse_help = "Reverses track from start timecode (in seconds) to end timecode (in seconds)";

	//load external modifiers help 
	this->import_functions_help = "";
	
	//loading libraries
	std::string ext(".dll");
	std::vector <std::filesystem::path> mods;

	for (auto& p : std::filesystem::recursive_directory_iterator(path_to_mods))
		if (p.path().extension() == ext) 
			mods.push_back(p.path());

	//uploading new function
	HINSTANCE temp_mod_handle = NULL;
	for (int i = 0; i < mods.size(); i++)
	{
		temp_mod_handle = LoadLibrary((LPCWSTR)mods[i].c_str());
		if (temp_mod_handle == NULL) 
		{
			std::cerr << "ERROR! " << mods[i] << " has failed to load!\n"; 
			this->mod_handles.emplace_back(nullptr);
		}
		else 
			this->mod_handles.emplace_back(temp_mod_handle);
		temp_mod_handle = NULL;
	}

	//getting help function
	h_func help_function;
	for (int i = 0; i < this->mod_handles.size(); i++)
	{
		if (this->mod_handles[i] == nullptr) 
			continue; //the whole thing with emplacing with nullptr was to remove the possibility to get wrong module into help because some previous wasn't loaded
		help_function = (h_func)GetProcAddress(this->mod_handles[i], "functions_help");
		if (!help_function) 
		{
			std::cerr << "Help wasn't found for " << mods[i].stem().string() + ".dll" << " module!\n"; 
			continue; 
		}
		this->import_functions_help += mods[i].stem().string() + +" module:\n" + help_function() + "\n\n";
	}

	//unloading libraries
	for (int i = 0; i < this->mod_handles.size(); i++)
	{
		if (this->mod_handles[i] != nullptr) FreeLibrary(this->mod_handles[i]);
	}
}

void Help::get_functions_config_file_help()
{
	std::cout << "Available functions for converter:\n";
	std::cout << "Please note that you don't need to print < > when you launch the program\n\n";

	std::cout << this->using_mute_help << "\n";
	std::cout << this->functionality_mute_help << "\n\n";

	std::cout << this->using_mix_help << "\n";
	std::cout << this->functionality_mix_help << "\n\n";

	std::cout << this->using_reverse_help << "\n";
	std::cout << this->functionality_reverse_help << "\n\n";

	std::cout << this->import_functions_help << "\n\n";

	std::cout << "Type \"BACK\" to go back or \"QUIT\" to close the program. \n";
}

void Help::get_launching_program_help()
{
	std::cout << "\tHow to launch Program:\n";
	std::cout << "Please note that you don't need to print < > when you launch the program\n\n";
	std::cout << "To launch program, you need to enter 5 arguments:\n";
	std::cout << "Mode for program to work in. You are currently in a HELP mode. \nTo launch it, you should print \"-h\" after program. You don't need to actually print anything else.\n";
	std::cout << "Type \"-c\" to get the program to a main working mode. \nIn this mode program will create a new file with modified sound!\n";
	std::cout << "\t<path to a config file> \n";
	std::cout << "\t<path to an output file> - in a .wav extension\n";
	std::cout << "\t<path to an input file> -  in a .wav extension\n";
	std::cout << "An array to a secondary input files. You will need these for some converters.\n";
	std::cout << "It can be done like this: [<input1.wav> <input2.wav> etc]\n";
	std::cout << "Please avoid cirillic letters and spaces in the path to your files. Otherwise an error may occur!\n\n";
	std::cout << "Type \"BACK\" to go back or \"QUIT\" to close the program.\n";
}

void Help::get_coding_extesions_help()
{
	std::cout << "\tWriting Extensions for Program:\n" ;
	std::cout << "Please note that you don't need to print < > when you launch the program\n\n";
	std::cout << "To write an extension you will have to have three things in your .dll file:\n";
	std::cout << "A structure of a .wav file header which should look like this:\n";
	std::cout << "struct Header {\n";
	std::cout << "\tchar chunkID[4];\n\tDWORD chunkSize;\n\tchar format[4];\n\tchar subchunk1ID[4];\n\tDWORD subchunk1Size;\n\tWORD audioFormat;\n\tWORD channelsNumber;\n\tDWORD sampleRate;\n\tDWORD byteRate;\n\tWORD bytesPerSample;\n\tWORD bitsPerSample;\n\tchar subchunk2ID[4];\n\tDWORD subchunk2Size;\n\tint DATA_SECTION_CURSOR_POS;\n\tint end_seconds_timecode;\n";
	std::cout << "}\n\n";
	std::cout << "Where \"DATA_SECTION_CURSOR_POS\" variable stands for position,\n where the start of the file contents(header excluded) is lying\n";
	std::cout << "And \"end_seconds_timecode\" variable stands for timecode of the end of file in seconds\n";
	std::cout << "Besides, you will need to have two functions: \nthe functional one (can be more than one) \nand the help functions which contains all the information about these functions:\n";
	std::cout << "Syntax and Functionality described.\n";
	std::cout << "Functions format:\n";
	std::cout << "\tvoid example_func(FILE* stream, Header streamHead, int start_timecode, int end_timecode)\n";
	std::cout << "\tstd::string func_help()\n\n";
	std::cout << "Type \"BACK\" to go back or \"QUIT\" to close the program.\n";
}

void Help::get_config_file_set_help()
{
	std::cout << "\tConfig file setting up help:\n";
	std::cout << "Config file is a text file in a program directory\n";
	std::cout << "You can set it up using commands\n";
	std::cout << "You can only set one action per line\n";
	std::cout << "Lines that are starting with \"#\" are commentary lines\n";
	std::cout << "In other lines you can set up a function with timecodes and arguments\n";
	this->get_functions_config_file_help();
}
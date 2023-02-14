//"E:/Documents/OOP/Sound_Processor/parse_test.txt" - main config file path
//"example music/output.wav" - output file
//"example music/severe_tire_damage.wav" - input file

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <queue>
#include "parser.h"
#include "Help.h"

enum help_state { MAIN_MENU, CONFIG, LAUNCH, HOW_TO };

int main(int argc, char* argv[])
{
	std::vector<std::string> secondary_input_files;
	secondary_input_files.push_back("example_music/funkorama.wav");

	std::string global_path_to_folder = "E:/Documents/OOP/Sound_Processor/";
	std::string mode; //program operation mode: "-c" - start and make modified file, "-h" - call the help console
	std::string config;
	std::string output_file;
	std::string main_stream_input_file;
	std::string secondary_streams_input_files;

	if (argc > 1) mode = argv[1]; //checking whether arguments are entered at startup
	else {
		std::cerr << "ERROR! \nNo arguments!\n"; 
		return -1;
	}

	if (mode == "-c") //normal operating mode
	{
		if (argc < 6)
		{ 
			std::cerr << "ERROR! Not enough arguments! Launch HELP menu (\"-h\") for help!\n"; 
			return -1;
		}
		else 
			if (argc > 6) 
				{ 
				std::cerr << "ERROR! Too many arguments! Launch HELP menu (\"-h\") for help!\n" << argv[6] <<"\n";
				return -1; 
			}

		//parsing input into the necessary arguments
		config = global_path_to_folder + argv[2];
		output_file = global_path_to_folder + argv[3];
		main_stream_input_file = global_path_to_folder + argv[4];
		secondary_streams_input_files = argv[5];
		secondary_streams_input_files = secondary_streams_input_files.substr(1, secondary_streams_input_files.size() - 2); //раскрываем скобки
		secondary_input_files = split(secondary_streams_input_files, ',');
		for (int i = 0; i < secondary_input_files.size(); i++)
		{
			secondary_input_files[i] = global_path_to_folder + secondary_input_files[i];
		}

	}
	else if (mode == "-h") //help console mode
	{
		system("cls");
		Help helper(global_path_to_folder + "mods/");
		help_state state = help_state::MAIN_MENU;
		std::string answer = "";

		while (true)
		{
			switch (state)
			{
			case MAIN_MENU:
				system("cls");
				std::cout << "Help\n";
				std::cout << "You are in the MAIN HELP MENU\n";
				std::cout << "Type the number function which you want:\n";
				std::cout << "\t1: General program launching help\n";
				std::cout << "\t2: Config file setting help\n";
				std::cout << "\t3: Writing extensions help\n\n";
				std::cout << "Type \"QUIT\" to close the program\n";
				std::cin >> answer;

				if (answer[0] == '1')
					state = help_state::LAUNCH;
				if (answer[0] == '2')
					state = help_state::CONFIG;
				if (answer[0] == '3')
					state = help_state::HOW_TO;
				if (answer == "QUIT") 
					return 0;

				break;

			case LAUNCH:
				system("cls");
				helper.get_launching_program_help();
				std::cin >> answer;
				if (answer == "BACK")
					state = help_state::MAIN_MENU;
				if (answer == "QUIT") 
					return 0;

				break;

			case CONFIG:
				system("cls");
				helper.get_config_file_set_help();
				std::cin >> answer;
				if (answer == "BACK") 
					state = help_state::MAIN_MENU;
				if (answer == "QUIT") 
					return 0;

				break;

			case HOW_TO:
				system("cls");
				helper.get_coding_extesions_help();
				std::cin >> answer;
				if (answer == "BACK") 
					state = help_state::MAIN_MENU;
				if (answer == "QUIT")
					return 0;

				break;
			}
		}

	}
	else //error in case of an incorrectly selected mode
	{
		std::cerr << "INCORRECT COMMAND! \nTry \"-c\" for launching program or \"-h\" to launch HELP MENU!\n";
		return -1;
	}

	//parsing input arguments
	FILE* input;
	input = fopen(main_stream_input_file.c_str(), "rb");

	std::vector <FILE*> secondary_inputs;
	FILE* temp = NULL;
	for (int i = 0; i < secondary_input_files.size(); i++)
	{
		temp = fopen(secondary_input_files[i].c_str(), "rb");
		if (temp == NULL) 
		{
			std::cout << "ERROR!\nFailed to open file " << secondary_input_files[i] << "!\n"; 
			return -1;
		}
		secondary_inputs.push_back(temp);
		temp = NULL;
	}


	//open all the necessary files
	FILE* output;
	output = fopen(output_file.c_str(), "w+b");

	//copy the first and the main stream to the output file
	Header inputHead;
	inputHead.parse_header(input);
	inputHead.write_header(output);
	copy_contents(input, output, inputHead.sampleRate * 2, inputHead); 

	//reading the headers of all input streams
	std::vector<Header> secondary_input_headers;
	for (int i = 0; i < secondary_input_files.size(); i++)
	{
		Header temp;
		temp.parse_header(secondary_inputs[i]);
		secondary_input_headers.push_back(temp);
	}


	//parse the configuration file and get a queue of modifiers with timecodes
	parser pars;
	pars.parse(config, &secondary_inputs, &secondary_input_headers);

	//apply modifiers to the output file
	//It is important to note that since we have already set all the parameters for the IMPORT modifier
	//and the functions are imported during the call to the convert function, we can limit ourselves to an easy call for
	//convert function. The same applies to the mix function
	while (!pars.modifiers_queue.empty())
	{
		std::tuple<WAVfileConverter*, int, int> b = pars.modifiers_queue.front();
		int end_timecode = std::get<2>(b);
		if (end_timecode == -1) end_timecode = std::get<0>(b)->second_stream_header->end_seconds_timecode;
		std::get<0>(b)->convert(output, inputHead, std::get<1>(b), end_timecode);
		delete std::get<0>(b);
		pars.modifiers_queue.pop();
	}

	//closing the streams
	fclose(input);
	for (int i = 0; i < secondary_input_files.size(); i++)
	{
		fclose(secondary_inputs[i]);
	}
	fclose(output);

	std::cout << "Done!\n";
}


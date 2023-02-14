#include "parser.h"

std::vector<std::string> split(std::string input_str, char separator);

void parser::parse(std::string input_path, std::vector<FILE*> *si, std::vector<Header> *sih) //MainFileHeader - header of the main file. The one being changed
{
#pragma push_macro("max")
#undef max //for numeric_limits::max()
	std::ifstream input(input_path);
	std::map <std::string, Converter> line_to_converter;
	line_to_converter["mute"] = Converter::MUTE;
	line_to_converter["mix"] = Converter::MIX;
	line_to_converter["reverse"] = Converter::REVERSE;

	if (!input.is_open()) 
	{
		std::cout << "An ERROR! has occured while trying to open file!\n";
		return;
	}

	std::string line;
	while (true)
	{
		std::getline(input, line, '\n');
		if (input.eof()) 
			break;
		if (line[0] == '#' || line.size() == 0)
			continue;
		else
		{
			int keypoint = this->modifiers_queue.size();
			std::vector<std::string> args = split(line, ' ');
			Converter mod;
			if (line_to_converter.find(args[0]) != line_to_converter.end())
				mod = line_to_converter[args[0]];
			else
			{
				std::cout << "Couldn't access default modifier with name " << args[0] << "! Set as an external mod...\n";
				WAVfileConverter* temp_pointer = WAVfileConverter::CreateNewConverter(Converter::IMPORT); //temporary pointer to set up converter
				temp_pointer->set_module_handle_and_function_name_with_string(args[0]);
				if (args.size() < 3) 
				{
					std::cout << "ERROR! \nMinimal argument count wasn't set!\n";
					return; 
				}

				int timecode_start = std::stoi(args[1]), timecode_end = std::stoi(args[2]);
				modifiers_queue.push(std::make_tuple(temp_pointer, timecode_start, timecode_end));
				continue;
			}
			switch (mod)
			{
			case Converter::MUTE:
				if (args.size() < 3) 
				{ 
					std::cout << "ERROR! \nMinimal argument count wasn't set!\n"; 
					return; 
				}
				modifiers_queue.push(std::make_tuple(WAVfileConverter::CreateNewConverter(Converter::MUTE), std::stoi(args[1]), std::stoi(args[2])));
				break;

			case Converter::MIX:
				if (args.size() == 4)
				{
					WAVfileConverter* temp_ptr = WAVfileConverter::CreateNewConverter(Converter::MIX); //temporary pointer to set up converter
					int stream_index = std::stoi(args[1].erase(0, 1));
					if (stream_index - 2 > (*si).size()) 
					{
						std::cout << "ERROR! \nTrying to reach a non existent stream at: " << line << "\n";
						return; 
					}
					temp_ptr->set_second_stream((*si)[stream_index - 2], &(*sih)[stream_index - 2]); //secondary input streams start with 2
					temp_ptr->is_second_stream_initialised = 1;
					modifiers_queue.push(std::make_tuple(temp_ptr, std::stoi(args[2]), std::stoi(args[3])));
				}
				else 
					if (args.size() == 3)
				{
					WAVfileConverter* temp_ptr = WAVfileConverter::CreateNewConverter(Converter::MIX); //temporary pointer to set up converter
					int stream_index = std::stoi(args[1].erase(0, 1));
					if (stream_index - 2 > (*si).size())
					{ 
						std::cout << "ERROR! \nTrying to reach a non existent stream at: " << line << "\n";
						return; 
					}
					temp_ptr->set_second_stream((*si)[stream_index - 2], &(*sih)[stream_index - 2]); //secondary input streams start with 2
					temp_ptr->is_second_stream_initialised = 1;
					modifiers_queue.push(std::make_tuple(temp_ptr, std::stoi(args[2]), -1));
				}
					else 
					{ 
						std::cout << "ERROR! \nMinimal argument count wasn't set!" << std::endl;
						return;
					}
				break;

			case Converter::REVERSE:
				if (args.size() < 3) 
				{ 
					std::cout << "Error! Minimal argument count wasn't set!\n"; 
					return;
				}
				modifiers_queue.push(std::make_tuple(WAVfileConverter::CreateNewConverter(Converter::REVERSE), std::stoi(args[1]), std::stoi(args[2])));
				break;
			}
		}
	}

	input.close();
#pragma pop_macro("max")
}
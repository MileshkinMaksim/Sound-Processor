#include "WAVfileConverter.h"

void WAVfileConverter::convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode) {}

WAVfileConverter* WAVfileConverter::CreateNewConverter(Converter converterID)
{
	WAVfileConverter* converter;

	switch (converterID)
	{
	case Converter::MUTE:
		converter = new MuteConverter();
		break;
	case Converter::MIX:
		converter = new MixConverter();
		break;
	case Converter::REVERSE:
		converter = new ReverseConverter();
		break;
	case Converter::IMPORT: 
		converter = new ImportConverter();
		break;
	default:
		converter = nullptr;
		break;
	}

	return converter;
}


void WAVfileConverter::set_module_handle_and_function_name_with_string(std::string function_full_name)
{
	int division_symbol_index = function_full_name.find(":", 0);
	if (division_symbol_index == -1) 
	{
		std::cout << "ERROR for " << function_full_name << ": Function name and module name weren't set proper name!\n";
		return; 
	}

	std::string module_name = "E:\\Documents\\OOP\\Sound_Processor\\mods" + function_full_name.substr(0, division_symbol_index) + ".dll";
	std::string func_name = function_full_name.substr(division_symbol_index + 1, function_full_name.length());

	HINSTANCE module_ptr = LoadLibrary((LPCWSTR)module_name.c_str());
	if (module_ptr == NULL) { std::cout << "Error! Couldn't load " << module_name << " module!" << std::endl << "Error id: " << GetLastError() << std::endl;  return; }

	this->module_handle = module_ptr;
	this->function_name = (LPCSTR)func_name.c_str();
}

void WAVfileConverter::set_second_stream(FILE* stream, Header* streamHead)
{
	this->second_stream = stream;
	this->second_stream_header = streamHead;
}

//Muting samples, finding the right ones and zeroing them out
void MuteConverter::convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode)
{
	//Checking the correctness of timecodes
	if (start_timecode > end_timecode)
	{ 
		printf("ERROR:\nInvalid timecodes for mute!\n");  
		return;
	}
	if (end_timecode > streamHead.end_seconds_timecode)
	{ 
		printf("ERROR:\nInvalid end timecode for mute!\n");
		return; 
	}
	if (start_timecode < 0) 
	{
		printf("ERROR:\nInvalid start timecode for mute!\n"); 
		return; 
	}

	int samples_count_to_mute = (end_timecode - start_timecode) * streamHead.sampleRate;
	fseek(stream, streamHead.DATA_SECTION_CURSOR_POS + start_timecode * streamHead.sampleRate * streamHead.bytesPerSample, SEEK_SET);

	WORD* zero = (WORD*)calloc(samples_count_to_mute, 2);
	fwrite(zero, streamHead.bytesPerSample, samples_count_to_mute, stream);
	delete zero;
}

//Mixing files by adding samples 
void MixConverter::convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode)
{

	if (this->is_second_stream_initialised == -1) 
	{
		printf("ERROR:\nadditional stream for mix wasn't initialised!\n"); 
		return; 
	}
	if (start_timecode > end_timecode) 
	{
		printf("ERROR:\nInvalid timecodes for mix!\n");  
		return; 
	}
	if (end_timecode > streamHead.end_seconds_timecode || end_timecode > this->second_stream_header->end_seconds_timecode) 
	{
		printf("Error: Invalid end timecode for mix!\n"); 
		return;
	}
	if (start_timecode < 0) 
	{ 
		printf("ERROR:\nInvalid start timecode for mix!\n");
		return; 
	}

	int samples_count = (end_timecode - start_timecode) * streamHead.sampleRate;
	fseek(stream, streamHead.DATA_SECTION_CURSOR_POS + start_timecode * streamHead.sampleRate * streamHead.bytesPerSample, SEEK_SET);

	WORD* first_input = new WORD[samples_count];
	fread(first_input, 2, samples_count, stream);
	fseek(stream, streamHead.DATA_SECTION_CURSOR_POS + start_timecode * streamHead.sampleRate * streamHead.bytesPerSample, SEEK_SET);

	WORD* second_input = new WORD[samples_count];
	fseek(this->second_stream, this->second_stream_header->DATA_SECTION_CURSOR_POS + start_timecode * this->second_stream_header->sampleRate * this->second_stream_header->bytesPerSample, SEEK_SET);
	fread(second_input, 2, samples_count, this->second_stream);

	int max_amplitude_value = (2 << (this->second_stream_header->bytesPerSample * 8)) - 1;
	for (int i = 0; i < samples_count; i++)
	{
		first_input[i] = first_input[i] + second_input[i];
		if (first_input[i] > max_amplitude_value) 
			first_input[i] = max_amplitude_value; //limiting
		else 
			if (first_input[i] < -max_amplitude_value) 
				first_input[i] = -max_amplitude_value;
	}

	fwrite(first_input, 2, samples_count, stream);
	delete[] first_input;
	delete[] second_input;
}

//Audio file reverse
void ReverseConverter::convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode) 
{
	if (start_timecode > end_timecode) 
	{ 
		printf("ERROR:\nInvalid timecodes for reverse!\n"); 
		return; 
	}
	if (end_timecode > streamHead.end_seconds_timecode) 
	{
		printf("ERROR:\nInvalid end timecode for reverse!\n"); 
		return;
	}
	if (start_timecode < 0) 
	{ 
		printf("ERROR:\nInvalid start timecode for reverse!\n");
		return;
	}

	int samples_count_to_reverse = (end_timecode - start_timecode) * streamHead.sampleRate;
	fseek(stream, streamHead.DATA_SECTION_CURSOR_POS + start_timecode * streamHead.sampleRate * streamHead.bytesPerSample, SEEK_SET);

	WORD* rev = new WORD[samples_count_to_reverse];
	fread(rev, 2, samples_count_to_reverse, stream);
	std::reverse(rev, rev + samples_count_to_reverse);
	fseek(stream, streamHead.DATA_SECTION_CURSOR_POS + start_timecode * streamHead.sampleRate * streamHead.bytesPerSample, SEEK_SET);
	fwrite(rev, 2, samples_count_to_reverse, stream);
	delete[] rev;
}

typedef void (__stdcall* f_func)(FILE*, Header, int, int);


void ImportConverter::convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode)
{
	if (this->module_handle == nullptr) { std::cout << "Error! DLL module handle wasn't initialised!" << std::endl; return; }
	if (this->function_name == nullptr) { std::cout << "Error! DLL importing function wasn't initialised!" << std::endl; return; }

	f_func func = (f_func)GetProcAddress(this->module_handle, this->function_name);
	if (!func) { std::cout << "Error! Function " << this->function_name << " couldn't be located!" << std::endl; return; }

	func(stream, streamHead, start_timecode, end_timecode);
	FreeLibrary(this->module_handle);
}


void WAVfileConverter::show() {}
void MuteConverter::show() 
{ 
	std::cout << "mute \n";
}
void MixConverter::show() 
{ 
	std::cout << "mix \n";
}
void ReverseConverter::show() 
{ 
	std::cout << "reverse \n"; 
}
void ImportConverter::show()
{
	std::cout << "import \n";
}
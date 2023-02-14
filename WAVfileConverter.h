#pragma once
#include <malloc.h>
#include <algorithm>
#include <string>
#include "WaveHeader.h"

enum Converter
{
	MUTE,
	MIX,
	REVERSE,
	IMPORT 
};


class WAVfileConverter
{
public:
	FILE* second_stream = nullptr;
	Header* second_stream_header = nullptr;
	HMODULE module_handle = nullptr;
	LPCSTR function_name = nullptr;
	int is_second_stream_initialised = -1;

	std::string using_help;
	std::string functionality_help;


	virtual void convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode);

	virtual void show();

	void set_second_stream(FILE* stream, Header* streamHead); //loads a second stream to use

	void set_module_handle_and_function_name_with_string(std::string function_full_name);//Blank for the imported function

	static WAVfileConverter* CreateNewConverter(Converter converterID);//creating converters using the fabric method
};

class MuteConverter : public WAVfileConverter
{
public:
	void convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode);
	void show();
};

class MixConverter : public WAVfileConverter
{
public:
	void convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode);
	void show();

};

class ReverseConverter : public WAVfileConverter
{
public:
	void convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode);
	void show();
};

class ImportConverter : public WAVfileConverter 
{
public:
	void convert(FILE* stream, Header streamHead, int start_timecode, int end_timecode);
	void show();
};

inline void copy_contents(FILE* input, FILE* output, int sample_capacity, Header input_header) //sample capacity - how much samples will be transferred after one iteration
//made to control memory usage
{
	WORD* sample_transfer = new WORD[sample_capacity];

	while (true)
	{
		int read = fread(sample_transfer, input_header.bytesPerSample, sample_capacity, input);
		if (read != sample_capacity)
		{
			fwrite(sample_transfer, input_header.bytesPerSample, read, output);
			break;
		}
		fwrite(sample_transfer, input_header.bytesPerSample, sample_capacity, output);
	}
	delete[] sample_transfer;
}
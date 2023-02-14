#pragma once
#include <windows.h>
#include <cstdio>
#include <iostream>
struct Header
{
	char chunkID[4];
	DWORD chunkSize; 
	char format[4];
	char subchunk1ID[4];
	DWORD subchunk1Size;
	WORD audioFormat;
	WORD channelsNumber;
	DWORD sampleRate;
	DWORD byteRate;
	WORD bytesPerSample;
	WORD bitsPerSample;
	char subchunk2ID[4];
	DWORD subchunk2Size;

	int DATA_SECTION_CURSOR_POS;
	int end_seconds_timecode;

	void try_fread(void* buffer, size_t size, size_t count, FILE* stream, std::string error_milestone_name)
	{
		if (fread(buffer, size, count, stream) != count)
		{
			if (feof(stream)) 
				printf("Premature end of file!\n");
			else 
				printf("File ERROR! while reading %s!\n", error_milestone_name);
		}
	}

	void try_fwrite(void* buffer, size_t size, size_t count, FILE* stream, std::string error_milestone_name)
	{
		if (fwrite(buffer, size, count, stream) != count)
			printf("ERROR! while writing file at writing %s!\n", error_milestone_name);
	}

	//WAV file parser for working with it
	void parse_header(FILE* input)
	{
		try_fread(chunkID, 1, 4, input, "RIFF\n");
		try_fread(&chunkSize, 4, 1, input, "chunk size\n");
		try_fread(format, 1, 4, input, "WAVE\n");
		try_fread(subchunk1ID, 1, 4, input, "fmt\n");
		try_fread(&subchunk1Size, 4, 1, input, "subchunk1 size\n");
		try_fread(&audioFormat, 2, 1, input, "audio format\n");
		try_fread(&channelsNumber, 2, 1, input, "channels number\n");
		try_fread(&sampleRate, 4, 1, input, "sample rate\n");
		try_fread(&byteRate, 4, 1, input, "byte rate\n");
		try_fread(&bytesPerSample, 2, 1, input, "bytes per sample\n");
		try_fread(&bitsPerSample, 2, 1, input, "bits per sample\n");
		try_fread(subchunk2ID, 1, 4, input, "subchunk2 ID. Presumably the \"data\"\n");
		while (strncmp(subchunk2ID, "data", 4) != 0) //the point is to omit all subchains before the main contents
		{
			DWORD infoSubchunkSize;
			try_fread(&infoSubchunkSize, 4, 1, input, "size of info subchunk\n");
			if (fseek(input, infoSubchunkSize, SEEK_CUR) != 0)
			{
				printf("ERROR! while setting cursor position!\n");
				return;
			}
			else 
				try_fread(subchunk2ID, 1, 4, input, "subchunk2 ID. Presumably the \"data\"\n");
		}
		try_fread(&subchunk2Size, 4, 1, input, "subchunk2 Size. File contents length\n");
		DATA_SECTION_CURSOR_POS = ftell(input);
		end_seconds_timecode = subchunk2Size / (sampleRate);
	}

	//Creating a WAV file header
	void write_header(FILE* output)
	{
		try_fwrite(chunkID, 1, 4, output, "RIFF\n");
		try_fwrite(&chunkSize, 4, 1, output, "chunk size\n");
		try_fwrite(format, 1, 4, output, "WAVE\n");
		try_fwrite(subchunk1ID, 1, 4, output, "fmt\n");
		try_fwrite(&subchunk1Size, 4, 1, output, "subchunk1 size\n");
		try_fwrite(&audioFormat, 2, 1, output, "audio format\n");
		try_fwrite(&channelsNumber, 2, 1, output, "channels number\n");
		try_fwrite(&sampleRate, 4, 1, output, "sample rate\n");
		try_fwrite(&byteRate, 4, 1, output, "byte rate\n");
		try_fwrite(&bytesPerSample, 2, 1, output, "bytes per sample\n");
		try_fwrite(&bitsPerSample, 2, 1, output, "bits per sample\n");
		try_fwrite(subchunk2ID, 1, 4, output, "subchunk2 ID. Presumably the \"data\"\n");
		try_fwrite(&subchunk2Size, 4, 1, output, "subchunk2 Size. File contents length\n");
	}

	//Data output for output of the WAV file header
	void show_nchar_array(int n, char* arr)
	{
		for (int i = 0; i < n; i++)
		{
			printf("%c", arr[i]);
		}
		printf("\n");
	}

	//Output of the WAV file header
	void show_info()
	{
		printf("chunkID:\t\t"); show_nchar_array(4, chunkID);
		printf("chunk size:\t\t%d\n", chunkSize);
		printf("format:\t\t\t"); show_nchar_array(4, format);
		printf("subchunk1 ID:\t\t"); show_nchar_array(4, subchunk1ID);
		printf("subchunk1 size:\t\t%d\n", subchunk1Size);
		printf("audio format:\t\t%d\n", audioFormat);
		printf("channels number:\t%d\n", channelsNumber);
		printf("sample rate:\t\t%d\n", sampleRate);
		printf("byte rate:\t\t%d\n", byteRate);
		printf("bytes per sample:\t%d\n", bytesPerSample);
		printf("bits per sample:\t%d\n", bitsPerSample);
		printf("subchunk2 ID:\t\t"); show_nchar_array(4, subchunk2ID);
		printf("subchunk2 size:\t\t%d\n",subchunk2Size);
		printf("cursor position for data section\t%d\n", DATA_SECTION_CURSOR_POS);
		printf("seconds:\t\t%d\n", end_seconds_timecode);
	}
};
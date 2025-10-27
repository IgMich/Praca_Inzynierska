#include "wavformat.h"

bool LoadWav(const char *filename, sound_t *sound) {
	bool return_value = true;
	FILE *file;
	char magic[4];
	int32_t filesize;
	int32_t format_length;		// 16
	int16_t format_type;		// 1 = PCM
	int16_t num_channels;		// 1
	int32_t sample_rate;		// 44100
	int32_t bytes_per_second;
	int16_t block_align;		// num_channels * bits_per_sample / 8
	int16_t bits_per_sample;	// 16
	int32_t data_size;

	file = fopen(filename, "rb");
	if(file == NULL) {
		PRINT_ERROR("%s: Failed to open file", filename);
		return false;
	}

	fread(magic, 1, 4, file);
	if(magic[0] != 'R' || magic[1] != 'I' || magic[2] != 'F' || magic[3] != 'F') {
		PRINT_ERROR("%s First 4 bytes should be \"RIFF\", are \"%4s\"", filename, magic);
		return_value = false;
		goto CLOSE_FILE;
	}

	fread(&filesize, 4, 1, file);

	fread(magic, 1, 4, file);
	if(magic[0] != 'W' || magic[1] != 'A' || magic[2] != 'V' || magic[3] != 'E') {
		PRINT_ERROR("%s 4 bytes should be \"WAVE\", are \"%4s\"", filename, magic);
		return_value = false;
		goto CLOSE_FILE;
	}

	fread(magic, 1, 4, file);
	if(magic[0] != 'f' || magic[1] != 'm' || magic[2] != 't' || magic[3] != ' ') {
		PRINT_ERROR("%s 4 bytes should be \"fmt/0\", are \"%4s\"", filename, magic);
		return_value = false;
		goto CLOSE_FILE;
	}

	fread(&format_length, 4, 1, file);
	fread(&format_type, 2, 1, file);
	if(format_type != 1) {
		PRINT_ERROR("%s format type should be 1, is %d", filename, format_type);
		return_value = false;
		goto CLOSE_FILE;
	}

	fread(&num_channels, 2, 1, file);
	if(num_channels != 2) {
		PRINT_ERROR("%s Number of channels should be 1, is %d", filename, num_channels);
		return_value = false;
		goto CLOSE_FILE;
	}

	fread(&sample_rate, 4, 1, file);
	if(sample_rate != 44100) {
		PRINT_ERROR("%s Sample rate should be 44100, is %d", filename, sample_rate);
		return_value = false;
		goto CLOSE_FILE;
	}

	fread(&bytes_per_second, 4, 1, file);
	fread(&block_align, 2, 1, file);
	fread(&bits_per_sample, 2, 1, file);
	if(bits_per_sample != 16) {
		PRINT_ERROR("%s bits per sample should be 16, is %d", filename, bits_per_sample);
		return_value = false;
		goto CLOSE_FILE;
	}

	fread(magic, 1, 4, file);
	if(magic[0] != 'd' || magic[1] != 'a' || magic[2] != 't' || magic[3] != 'a') {
		PRINT_ERROR("%s 4 bytes should be \"data\", are \"%4s\"", filename, magic);
		return_value = false;
		goto CLOSE_FILE;
	}

	fread(&data_size, 4, 1, file);

	sound->data = malloc(data_size);
	if(sound->data == NULL) {
		PRINT_ERROR("%s Failed to allocate %d bytes for data", filename, data_size);
		return_value = false;
		goto CLOSE_FILE;
	}

	if(fread(sound->data, 1, data_size, file) != data_size) {
		PRINT_ERROR("%s Failed to read data bytes", filename);
		return_value = false;
		free(sound->data);
		goto CLOSE_FILE;
	}

	sound->samples = data_size / 2;
	sound->bytes_per_second = bytes_per_second;

	CLOSE_FILE:
	fclose(file);
	printf("Sound samples: %d\n",sound->samples);
	printf("Bytes per second: %d\n",sound->bytes_per_second);

	return return_value;
}
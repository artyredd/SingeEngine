#include "io/file.h"
#include "singine/memory.h"

#define NotNull(variableName) if (variableName is null) { fprintf(stderr, #variableName"can not be null"); throw(InvalidArgumentException); }

bool TryOpen(const char* path, FileMode fileMode, File* out_file)
{
	// make sure to set the out value always to it's default value first
	*out_file = null;

	if (path is null || fileMode is null)
	{
		return false;
	}

	File file;

	errno_t error = fopen_s(&file, path, fileMode);

	if (file is null)
	{
		return false;
	}

	*out_file = file;

	return true;
}

File Open(const char* path, FileMode fileMode)
{
	NotNull(path);
	NotNull(fileMode);

	File file;

	errno_t error = fopen_s(&file, path, fileMode);

	if (file is null)
	{
		fprintf(stderr, "Failed to open the file %s ErrorCode: %i"NEWLINE, path, error);
		throw(FileNotFoundException);
	}

	return file;
}

size_t GetFileSize(const File file)
{
	NotNull(file);

	size_t currentPosition = ftell(file);

	// seek to the end of the file and store the length of the file
	fseek(file, 0L, SEEK_END);

	size_t count = ftell(file);

	// if we did not start at the beginning of the file we should return to the original position inside of the file
	if (currentPosition != 0)
	{
		fseek(file, (long)currentPosition, SEEK_SET);
	}

	return count + 1;
}

char* ReadFile(const File file)
{
	size_t length = GetFileSize(file);

	char* result = SafeAlloc(length);

	for (size_t i = 0; i < length; i++)
	{
		int c = fgetc(file);

		if (c is EOF)
		{
			fprintf(stderr, "An error occurred while reading the file at ptr: %llix, Error Code %i", (size_t)file, ferror(file));
			throw(FailedToReadFileException);
		}

		result[i] = c;
	}

	return result;
}

char* ReadAll(const char* path)
{
	File file;
	if (TryOpen(path, FileModes.Read, &file))
	{
		char* data = ReadFile(file);

		if (data is null)
		{
			fprintf(stderr, "Failed to read file %s"NEWLINE, path);
			throw(FailedToReadFileException);
		}

		if (TryClose(file) is false)
		{
			fprintf(stderr, "Failed to close the file %s"NEWLINE, path);
			throw(FailedToCloseFileException);
		}

		return data;
	}

	fprintf(stderr, "Failed to open file %s"NEWLINE, path);
	throw(FailedToOpenFileException);
}

bool TryClose(File file)
{
	return fclose(file) != EOF;
}

#undef NotNull
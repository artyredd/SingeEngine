#include "singine/file.h"
#include "singine/memory.h"
#include "singine/guards.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bool TryOpen(const char* path, FileMode fileMode, File* out_file);
static File Open(const char* path, FileMode fileMode);
static size_t GetFileSize(const File file);
static char* ReadFile(const File file);
static bool TryReadFile(const File file, char** out_data);
static char* ReadAll(const char* path);
static bool TryReadAll(const char* path, char** out_data);
static bool TryReadLine(File file, char* buffer, size_t offset, size_t bufferLength, size_t* out_lineLength);
static bool TryGetSequenceCount(File file, const char* targetSequence, const size_t targetLength, const char* abortSequence, const size_t abortLength, size_t* out_count);
static bool TryClose(File file);
static void Close(File file);
static bool TryVerifyCleanup(void);

const struct _fileMethods Files = {
	.TryOpen = &TryOpen,
	.Open = &Open,
	.GetFileSize = &GetFileSize,
	.ReadFile = &ReadFile,
	.TryReadFile = &TryReadFile,
	.ReadAll = &ReadAll,
	.TryReadAll = &TryReadAll,
	.TryReadLine = &TryReadLine,
	.TryGetSequenceCount = &TryGetSequenceCount,
	.TryClose = &TryClose,
	.Close = &Close,
	.TryVerifyCleanup = &TryVerifyCleanup
};

// the number of file handles opened using this file
size_t FILES_OPENED;
// the number of file handles closed using this file
size_t FILES_CLOSED;

static bool TryOpen(const char* path, FileMode fileMode, File* out_file)
{
	// make sure to set the out value always to it's default value first
	*out_file = null;

	if (path is null || fileMode is null)
	{
		return false;
	}

	File file;

#pragma warning(disable: 4189)
	errno_t error = fopen_s(&file, path, fileMode);
#pragma warning(default: 4189)
	if (file is null)
	{
		char buffer[1024];

		strerror_s(buffer, 1024, error);

		fprintf(stderr, "Error opening file %s: %s"NEWLINE, path, buffer);

		return false;
	}

	*out_file = file;

	++(FILES_OPENED);

	return true;
}

static File Open(const char* path, FileMode fileMode)
{
	GuardNotNull(path);
	GuardNotNull(fileMode);

	File file;

	errno_t error = fopen_s(&file, path, fileMode);

	if (file is null)
	{
		fprintf(stderr, "Failed to open the file %s ErrorCode: %i"NEWLINE, path, error);
		throw(FileNotFoundException);
	}

	++(FILES_OPENED);

	return file;
}

static size_t GetFileSize(const File file)
{
	GuardNotNull(file);

	size_t currentPosition = _ftelli64(file);

	// this isn't the most portable solution but I wrapped this in an abstraction incase I need to diversify the portability later

	// seek to the end of the file and store the length of the file
	if (_fseeki64(file, 0, SEEK_END) != 0)
	{
		throw(FailedToReadFileException);
	}

	size_t count = _ftelli64(file);

	// if we did not start at the beginning of the file we should return to the original position inside of the file
	if (currentPosition != 0)
	{
		rewind(file);

		if (_fseeki64(file, currentPosition, SEEK_SET) is 0)
		{
			throw(FailedToReadFileException);
		}
	}

	return count;
}

static char* ReadFile(const File file)
{
	size_t length = GetFileSize(file);

	char* result = SafeAlloc(length + 1);

	result[length] = '\0'; // add line terminator

	rewind(file);

	for (size_t i = 0; i < length; i++)
	{
		int c = fgetc(file);

		if (c is EOF)
		{
			// check to see what error we got
			// if we got 0 then the stream was likely not started at the beginning and we reached EOF early
			int error = ferror(file);

			// if the error we got was actually an error we should return early
			// only attempt rewinding once
			if (error != 0)
			{
				SafeFree(result);
				fprintf(stderr, "An error occurred while reading the file at ptr: %llix, Error Code %i", (size_t)file, ferror(file));
				throw(FailedToReadFileException);
			}

			result[i] = '\0';
			break;
		}

		result[i] = (char)c;
	}

	return result;
}

static bool TryReadFile(const File file, char** out_data)
{
	*out_data = null;

	size_t length = GetFileSize(file);

	char* result = SafeAlloc(length + 1);

	result[length] = '\0'; // add line terminator

	rewind(file);

	for (size_t i = 0; i < length; i++)
	{
		int c = fgetc(file);

		if (c is EOF)
		{
			// check to see what error we got
			// if we got 0 then the stream was likely not started at the beginning and we reached EOF early
			int error = ferror(file);

			// if the error we got was actually an error we should return early
			// only attempt rewinding once
			if (error != 0)
			{
				SafeFree(result);
				return false;
			}

			result[i] = '\0';
			break;
		}

		result[i] = (char)c;
	}

	*out_data = result;

	return true;
}

static char* ReadAll(const char* path)
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

	// we shouldn't be able to get here, but it's possible if __debugbreak() is continued
	return null;
}

static bool TryReadAll(const char* path, char** out_data)
{
	File file;

	if (TryOpen(path, FileModes.Read, &file))
	{
		char* data;

		if (TryReadFile(file, &data))
		{
			*out_data = data;

			return TryClose(file);
		}

		TryClose(file);
	}

	return false;
}

static bool TryReadLine(File file, char* buffer, size_t offset, size_t bufferLength, size_t* out_lineLength)
{
	GuardNotNull(file);

	char* result = fgets(buffer + offset, (int)(bufferLength - offset), file);

	// if eof is encountered before any characters
	if (result is null || ferror(file))
	{
		return false;
	}

	size_t length = strlen(result);

	if (result[length - 1] is '\n' || result[length - 1] is '\r')
	{
		--(length);
		result[length] = '\0';
	}

	if (result[length - 1] is '\n' || result[length - 1] is '\r')
	{
		--(length);
		result[length] = '\0';
	}

	length = max(length, 0);

	*out_lineLength = length;

	return true;
}

static bool TryGetSequenceCount(File file, const char* targetSequence, const size_t targetLength, const char* abortSequence, const size_t abortLength, size_t* out_count)
{
	GuardNotNull(targetSequence);
	GuardNotZero(targetLength);
	GuardNotNull(abortSequence);
	GuardNotZero(abortLength);

	// store the position that the file currently is in
	long previousPosition = ftell(file);

	if (previousPosition is - 1L)
	{
		return false;
	}

	// read until either we encounter abortSequence or EOF
	size_t count = 0;
	size_t abortIndex = 0;
	size_t targetIndex = 0;

	int c;
	while ((c = fgetc(file)) != EOF)
	{
		// check to see if we are at the start of the target sequence
		if (c is abortSequence[abortIndex])
		{
			++abortIndex;

			// check to see if we have found the entire sequence
			if (abortIndex >= abortLength)
			{
				break;
			}
		}
		else
		{
			// if the current character is not part of the target reset 
			abortIndex = 0;
		}

		// check to see if we are at the start of the target sequence
		if (c is targetSequence[targetIndex])
		{
			++targetIndex;

			// check to see if we have found the entire sequence
			if (targetIndex >= targetLength)
			{
				++count;
				targetIndex = 0;
			}
		}
		else
		{
			// if the current character is not part of the target reset 
			targetIndex = 0;
		}
	}

	if (ferror(file))
	{
		return false;
	}

	// return to beginning
	if (fseek(file, previousPosition, SEEK_SET) != 0)
	{
		return false;
	}

	*out_count = count;

	return true;
}

static bool TryClose(File file)
{
	++(FILES_CLOSED);
	return fclose(file) != EOF;
}

static void Close(File file)
{
	if (TryClose(file) is false)
	{
		throw(FailedToCloseFileException);
	}
}

static bool TryVerifyCleanup(void)
{
	if (FILES_OPENED != FILES_CLOSED)
	{
		fprintf(stderr, "The number of the files that were opened did not match the number of file handles that were closed: %lli/%lli"NEWLINE, FILES_OPENED, FILES_CLOSED);
	}

	return FILES_OPENED == FILES_CLOSED;
}
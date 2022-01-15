#include "singine/conversions.h"
#include "singine/guards.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

void ToLower(char* buffer, size_t bufferLength, size_t offset)
{
	GuardNotNull(buffer);
	GuardNotZero(bufferLength);
	GuardLessThan(offset, bufferLength);

	for (size_t i = offset; i < bufferLength; i++)
	{
		buffer[i] = (char)tolower(buffer[i]);
	}
}

void ToUpper(char* buffer, size_t bufferLength, size_t offset)
{
	GuardNotNull(buffer);

	for (size_t i = offset; i < bufferLength; i++)
	{
		buffer[i] = (char)toupper(buffer[i]);
	}
}

size_t Trim(char* buffer, const size_t bufferLength)
{
	size_t startIndex = 0;
	size_t endIndex = bufferLength - 1;

	size_t index = 0;
	// traverse the string the first non-whitespace char we hit is the start of the string
	while (index < bufferLength and buffer[index] isnt '\0' and isspace(buffer[index])) index++;

	startIndex = index;

	// if the start index is the end index that mean the whole thing is whitespace return 0
	if (startIndex is endIndex)
	{
		buffer[0] = '\0';
		return 0;
	}

	for (size_t i = startIndex; i < bufferLength; i++)
	{
		int c = buffer[i];
		if (isspace(c) is false)
		{
			endIndex = i;
		}
	}

	// if there was nothing to trim return
	if (startIndex is 0 and endIndex is(bufferLength - 1))
	{
		return bufferLength;
	}

	// now we have the start and end indexes we should move the entire string to the beginning of the buffer
	size_t length = max(endIndex - startIndex, 0) + 1;
	memmove(buffer, buffer + startIndex, length);

	// null terminate
	buffer[length] = '\0';

	return length;
}
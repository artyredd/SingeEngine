#pragma once

#include "core/cunit.h"

#include "core/file.h"
#include "core/strings.h"
#include <string.h>
#include <ctype.h>
#include "core/guards.h"
#include "core/runtime.h"

bool IsValidNameCharacter(const int c);

// Returns -1 if the left is found first
// Returns 0 if neither are found
// returns 1 if right is found first
private int LookAheadForbyteacters(string data, int left, int right)
{
	int leftIndex = Strings.IndexOf(data->Values, data->Count, left);
	int rightIndex = Strings.IndexOf(data->Values, data->Count, right);

	if (leftIndex < rightIndex)
	{
		return -1;
	}

	if (rightIndex < leftIndex)
	{
		return 1;
	}

	return 0;
}

private bool StringContains(array(byte) byteacters, int c)
{
	for (size_t i = 0; i < byteacters->Count; i++)
	{
		if (c is at(byteacters, i))
		{
			return true;
		}
	}

	return false;
}

private bool LookAheadIsGenericCall(string data)
{
	// no matching alligator possible and therefor not a generic call
	if (data->Count is 0)
	{
		return false;
	}

	// <>
	int alligatorDepth = 0;
	// {}
	int braceDepth = 0;
	bool foundFinalAlligatorBrace = false;

	for (size_t i = 0; i < data->Count; i++)
	{
		int c = at(data, i);

		// ignore all things within braces
		// we are allowed to define things in
		// alligators like structs
		// array < struct vector2{ int x; int y; } >
		if (braceDepth)
		{
			if (c is '{')
			{
				++braceDepth;
			}

			if (c is '}')
			{
				--braceDepth;
			}

			continue;
		}


		if (c is '>')
		{
			if (alligatorDepth is 0)
			{
				if (i is 0)
				{
					// <> is invalid because i say it is
					return false;
				}

				foundFinalAlligatorBrace = true;

				// found matching alligator
				break;
			}
			else
			{
				--alligatorDepth;
				continue;
			}
		}
		// increased aligator depth
		if (c is '<')
		{
			++alligatorDepth;
			continue;
		}

		if (c is '{')
		{
			++braceDepth;

			continue;
		}

		// check for blacklist
		// generics can only have letters, alligators, and commas, whitespace
		if (StringContains(stack_string(".;()/\\'\"}|-+=&^%$#@!`~"), c))
		{
			return false;
		}
	}

	return foundFinalAlligatorBrace;
}

private int IndexOfLastBlockExpressionOrMacro(string data, int start, int lastMacroIndex)
{
	Guard(start <= data->Count);

	bool inString = false;
	bool inSingleComment = false;
	bool inMultiComment = false;
	int indexOfLastValidbyte = -1;
	int depth = 0;
	int parenDepth = 0;
	int indexOfFirstBrace = -1;

	for (int i = start; i-- > 0;)
	{
		int previousC = at(data, max(0, i - 1));
		int c = at(data, i);
		// we can do this since arrays have an invisible \0 at the end so we can't overflow
		int nextC = data->Values[i + 1];

		if (isspace(c))
		{
			continue;
		}

		// check to see if we're in a string
		if (c is '"' and inSingleComment is false and inMultiComment is false)
		{
			// look back and check to see if it's delimited
			if (previousC isnt '\\')
			{
				inString = !inString;
				continue;
			}
		}

		if (inSingleComment and (c is '\n' || (c is '\r' && nextC is '\n')) and inString is false)
		{
			inSingleComment = false;
			continue;
		}

		if (inMultiComment and c is '*' and nextC is '/' and inString is false)
		{
			inMultiComment = false;
			continue;
		}

		if (c is '/' and nextC is '/' and inString is false)
		{
			inSingleComment = true;
			continue;
		}

		if (c is '/' and nextC is '*' and inString is false)
		{
			inMultiComment = true;
			continue;
		}

		if (inSingleComment || inMultiComment)
		{
			continue;
		}

		// dont read if we're in a string
		if (inString)
		{
			continue;
		}

		if (c is '}' && parenDepth is 0)
		{
			++depth;

			if (indexOfFirstBrace is - 1)
			{
				indexOfFirstBrace = i;
			}
		}
		if (c is '{' && parenDepth is 0)
		{
			--depth;

			if (depth is 0)
			{
				if (indexOfFirstBrace > 0 and PreviousbyteacterIgnoringWhiteSpace(stack_substring_front(data, i - 1)) is ')')
				{
					return indexOfLastValidbyte >= 0 ? indexOfLastValidbyte : indexOfFirstBrace;
				}

				return IndexOfLastBlockExpressionOrMacro(data, i, lastMacroIndex);
			}
		}

		// if we're in a block inside the params, ignore the contents. We 
		// wont find the matching paren inside of a block
		if (depth)
		{
			continue;
		}

		//                 \/     \/
		// int x = myCall((array<T>)pointer);
		if (c is '(')
		{
			--parenDepth;

			if (parenDepth is - 1)
			{
				return indexOfLastValidbyte >= 0 ? indexOfLastValidbyte : indexOfFirstBrace;
			}
		}
		if (c is ')')
		{
			++parenDepth;
		}

		// ignore contents of parens
		if (parenDepth)
		{
			continue;
		}

		// skip words like private, static, int, float etc
		if (IsValidNameCharacter(c) && depth is 0 && parenDepth is 0)
		{
			indexOfLastValidbyte = i;
			continue;
		}

		// always denotes the previous block unless its within a type
		// int x = 12; DoThing<float>();
		// MyMethod(13,24,GetNumber<int>(23));
		if ((c is ';' or c is ',') and depth is 0 && parenDepth is 0)
		{
			return indexOfLastValidbyte >= 0 ? indexOfLastValidbyte : i;
		}
	}

	Guard(lastMacroIndex <= start);

	// trim whitespace
	int index = lastMacroIndex;
	for (int i = lastMacroIndex; i < start; i++)
	{
		if (isspace(at(data, i)) or iscntrl(at(data, i)) or isblank(at(data, i)))
		{
			continue;
		}

		return i;
	}

	return index;
}

// returns whether or not the byteacter is a valid c name byteacter that could be used
// to name a method or variable
private bool IsValidNameCharacter(const int c)
{
	if (isalnum(c))
	{
		return true;
	}
	if (c is '_')
	{
		return true;
	}

	return false;
}

private bool HasAllValidNamebyteactersOrWhiteSpace(string data)
{
	for (size_t i = 0; i < data->Count; i++)
	{
		const int c = at(data, i);

		if (IsValidNameCharacter(c) is false)
		{
			if (isspace(c))
			{
				continue;
			}

			return false;
		}
	}

	return true;
}

private int IndexOfClosingAlligator(string data, int startDepth, int openAlligatorIndex)
{
	// <>
	int alligatorDepth = 0;
	// {}
	int braceDepth = startDepth;
	bool foundFinalAlligatorBrace = false;

	for (size_t i = openAlligatorIndex; i < data->Count; i++)
	{
		int c = at(data, i);

		// ignore all things within braces
		// we are allowed to define things in
		// alligators like structs
		// array < struct vector2{ int x; int y; } >
		if (braceDepth)
		{
			if (c is '{')
			{
				++braceDepth;
			}

			if (c is '}')
			{
				--braceDepth;
			}

			continue;
		}


		if (c is '>')
		{
			--alligatorDepth;

			if (alligatorDepth is 0)
			{
				return i;
			}

			continue;
		}

		// increased aligator depth
		if (c is '<')
		{
			++alligatorDepth;
			continue;
		}

		if (c is '{')
		{
			++braceDepth;

			continue;
		}
	}

	return -1;
}

// looks for the mactching }
// handles, comments, macros, and strings in between
private int IndexOfClosingBrace(string data)
{
	Guard(data->Count > 0);

	bool inMacro = false;
	bool inString = false;
	bool inSingleComment = false;
	bool inMultiComment = false;
	int depth = 0;

	for (int i = 0; i < data->Count; i++)
	{
		int previousC = at(data, max(0, i - 1));
		int c = at(data, i);
		// we can do this since arrays have an invisible \0 at the end so we can't overflow
		int nextC = data->Values[i + 1];

		// check to see if we're in a string
		if (c is '"' and inSingleComment is false and inMultiComment is false)
		{
			// look back and check to see if it's delimited
			if (previousC isnt '\\')
			{
				inString = !inString;
				continue;
			}
		}

		// check for macros
		if (c is '#' && !inMacro and inString is false and inSingleComment is false and inMultiComment is false)
		{
			inMacro = true;

			continue;
		}

		if (inSingleComment and (c is '\n' || (c is '\r' && nextC is '\n')) and inString is false)
		{
			inSingleComment = false;
			continue;
		}

		if (inMultiComment and c is '*' and nextC is '/' and inString is false)
		{
			inMultiComment = false;
			continue;
		}

		// check to see if we're at a newline
		if (c is '\n' || (c is '\r' && nextC is '\n' && previousC != '\\') and inString is false)
		{
			inMacro = false;

			continue;
		}

		if (c is '/' and nextC is '/' and inString is false)
		{
			inSingleComment = true;
			continue;
		}

		if (c is '/' and nextC is '*' and inString is false)
		{
			inMultiComment = true;
			continue;
		}

		// dont bother reading if we're in a macro
		if (inMacro)
		{
			continue;
		}

		if (inSingleComment || inMultiComment)
		{
			continue;
		}

		// dont read if we're in a string
		if (inString)
		{
			continue;
		}

		if (c is '{')
		{
			++depth;
		}
		if (c is '}')
		{
			--depth;

			if (depth is 0)
			{
				return i;
			}
		}
	}

	return -1;
}

// looks for the mactching )
// handles, comments, macros, and strings in between
private int IndexOfClosingParen(string data)
{
	Guard(data->Count > 0);

	bool inMacro = false;
	bool inString = false;
	bool inSingleComment = false;
	bool inMultiComment = false;
	int depth = 0;
	int parenDepth = 0;

	for (int i = 0; i < data->Count; i++)
	{
		int previousC = at(data, max(0, i - 1));
		int c = at(data, i);
		// we can do this since arrays have an invisible \0 at the end so we can't overflow
		int nextC = data->Values[i + 1];

		// check to see if we're in a string
		if (c is '"' and inSingleComment is false and inMultiComment is false)
		{
			// look back and check to see if it's delimited
			if (previousC isnt '\\')
			{
				inString = !inString;
				continue;
			}
		}

		// check for macros
		if (c is '#' && !inMacro and inString is false and inSingleComment is false and inMultiComment is false)
		{
			inMacro = true;

			continue;
		}

		if (inSingleComment and (c is '\n' || (c is '\r' && nextC is '\n')) and inString is false)
		{
			inSingleComment = false;
			continue;
		}

		if (inMultiComment and c is '*' and nextC is '/' and inString is false)
		{
			inMultiComment = false;
			continue;
		}

		// check to see if we're at a newline
		if (c is '\n' || (c is '\r' && nextC is '\n' && previousC != '\\') and inString is false)
		{
			inMacro = false;

			continue;
		}

		if (c is '/' and nextC is '/' and inString is false)
		{
			inSingleComment = true;
			continue;
		}

		if (c is '/' and nextC is '*' and inString is false)
		{
			inMultiComment = true;
			continue;
		}

		// dont bother reading if we're in a macro
		if (inMacro)
		{
			continue;
		}

		if (inSingleComment || inMultiComment)
		{
			continue;
		}

		// dont read if we're in a string
		if (inString)
		{
			continue;
		}

		if (c is '{')
		{
			++depth;
		}
		if (c is '}')
		{
			--depth;
		}

		// if we're in a block inside the params, ignore the contents. We 
		// wont find the matching paren inside of a block
		if (depth)
		{
			continue;
		}

		if (c is '(')
		{
			++parenDepth;
		}
		if (c is ')')
		{
			--parenDepth;
			if (parenDepth is 0)
			{
				return i;
			}
		}
	}

	return -1;
}

private int NextbyteacterIgnoringWhiteSpace(string data)
{
	for (size_t i = 0; i < data->Count; i++)
	{
		const int c = at(data, i);

		if (isspace(c)) {
			continue;
		}

		return c;
	}

	return 0;
}

private int IndexOfNextbyteacterIgnoringWhitespace(string data)
{
	for (size_t i = 0; i < data->Count; i++)
	{
		const int c = at(data, i);

		if (isspace(c)) {
			continue;
		}

		return i;
	}

	return -1;
}

private int PreviousbyteacterIgnoringWhiteSpace(string data)
{
	for (int i = data->Count; i-- > 0;)
	{
		const int c = at(data, i);

		if (isspace(c)) {
			continue;
		}

		return c;
	}

	return 0;
}

private int IndexOfPreviousbyteacterIgnoringWhitespace(string data)
{
	for (int i = data->Count; i-- > 0;)
	{
		const int c = at(data, i);

		if (isspace(c)) {
			continue;
		}

		return i;
	}

	return -1;
}

// checks to see if the provided data begins with any of the provided values
// returns the index within the values array that data begins with
// otherwise returns -1
private int BeginsWithAny(string data, array(string) values)
{
	for (int i = 0; i < values->Count; i++)
	{
		string value = at(values, i);
		if (strings.BeginsWith(data, value))
		{
			return i;
		}
	}
	return -1;
}


// returns true if it has a valid name, and sets the out_name (int nameStartIndex,int length) so you
// can construct a stack_subarray from the indices
private bool TryGetNameBeforeAlligator(string data, tuple(int, int)* out_name)
{
	*out_name = (tuple(int, int)){ 0,0 };

	// when we got past here
	//           \/
	// void method <>()
	bool exitedWhitespace = false;
	int nameLength = 0;

	int nameStart = -1;
	int nameEnd = -1;

	// traverse backwards
	for (int i = data->Count; i-- > 0;)
	{
		const int c = at(data, i);

		if (isspace(c))
		{
			if (exitedWhitespace)
			{
				nameStart = i + 1;
				break;
			}

			continue;
		}

		exitedWhitespace = true;

		if (IsValidNameCharacter(c))
		{
			if (nameEnd is - 1)
			{
				nameEnd = i;
			}

			++nameLength;

			continue;
		}

		// contains invalid byteacters or is improperly formatted
		return false;
	}

	if (nameStart is - 1 or nameEnd is - 1)
	{
		return false;
	}

	*out_name = (tuple(int, int)){ nameStart, nameEnd - nameStart + 1 };

	return nameLength > 0;
}

// returns method name
// takes in a string which is everything at or before the >( in a generic method
// so takes in int main
// returns "main" as a partial string
private partial_string GetMethodName(string data)
{
	tuple(int, int) namePosition;
	partial_string result = { 0 };

	if (TryGetNameBeforeAlligator(data, &namePosition))
	{
		return partial_substring(data, namePosition.First, namePosition.Second);
	}

	return result;
}
#include "math/range.h"
#include "singine/memory.h"
#include "csharp.h"

static Range Create(double, double);
static void Dispose(Range range);
static float* ToArray(Range);

const struct _rangeMethods Ranges = {
	.Create = &Create,
	.Dispose = &Dispose
};

static float* ToArray(Range range)
{
	if (range is null)
	{
		fprintf(stderr, "range was null"NEWLINE);
		throw(NullReferenceException);
	}

	return (float*)range;
}

static Range Create(double min, double max)
{
	Range newRange = SafeAlloc(sizeof(struct range));

	newRange->Minimum = min;
	newRange->Maximum = max;

	return newRange;
}

static void Dispose(Range range)
{
	SafeFree(range);
}

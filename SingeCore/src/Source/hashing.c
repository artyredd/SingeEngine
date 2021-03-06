#include "singine/hashing.h"
#include "csharp.h"

static size_t Hash(const char* bytes);
static size_t ChainHash(const char* bytes, const size_t previousHash);

const struct _hashingMethods Hashing = {
	.Hash = &Hash,
    .ChainHash = &ChainHash
};

static size_t ChainHash(const char* bytes, const size_t previousHash)
{
    // modified djb2 hash
    /*http://www.cse.yorku.ca/~oz/hash.html*/

    size_t hash = previousHash is 0 ? 5381 : previousHash;

    int c;

    while (c = *bytes++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static size_t Hash(const char* bytes)
{
    return ChainHash(bytes, 0);
}
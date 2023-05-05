#pragma once
//implement a 81920 bits bloom filter with murmurhash3

#include "MurmurHash3.h"
#include <cstring>
#include <iostream>
typedef unsigned long long ull;
using namespace std;

void setBit(char* bloom_filter, int pos)
{
    int byte = pos / 8;
    int bit = pos % 8;
    bloom_filter[byte] |= (1 << (8 - bit - 1));
}

bool getBit(char* bloom_filter, int pos)
{
    int byte = pos / 8;
    int bit = pos % 8;
    return (bloom_filter[byte] & (1 << (8 - bit - 1))) != 0;
}

void add(char* bloom_filter, const ull* key)
{
    unsigned int hash[4] = { 0 };
    MurmurHash3_x64_128(key, sizeof(*key), 1, hash);
    for (int i = 0; i < 4; i++)
    {
        setBit(bloom_filter, hash[i] % (10240 * 8));
    }
}

bool contains(char* bloom_filter, const ull* key)
{
    unsigned int hash[4] = { 0 };
    MurmurHash3_x64_128(key, sizeof(*key), 1, hash);
    for (int i = 0; i < 4; i++)
    {
        if (!getBit(bloom_filter, hash[i] % (10240 * 8)))
        {
            return false;
        }
    }
    return true;
}
#include "SSTable.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include "BloomFilter.h"
using namespace std;

SSTable::SSTable()
{
    kv_number = 1;
    index = new pair<ull, int>[1];
    index[0] = { 1, 0 };
    data = new char[5];
    strncpy(data, "1234\0", 5);
}

SSTable::SSTable(const Skiplist &skiplist, ull _timestamp, const string& _dir, int level)
{
    this->level = level;
    //convert skiplist to SSTable
    dir = _dir;
    timestamp = _timestamp;
    kv_number = skiplist.size;
    index = new pair<ull, int>[kv_number];
    min_key = skiplist.head->forward[0]->key;
    memset(bloom_filter, 0, 10240);

    //add keys to bloom filter
    SkiplistNode* curr = skiplist.head->forward[0];
    while (curr) {
        add(bloom_filter, &curr->key);
        if(curr->forward[0])
            max_key = curr->forward[0]->key;
        curr = curr->forward[0];
    }

    //construct the pair<ull, int> index and char* data
    int offset = 0;
    curr = skiplist.head->forward[0];
    data = new char[0x200000 - 4 * 8 - 10240 - kv_number * sizeof(pair<ull, int>)];
    memset(data, 0, 0x200000 - 4 * 8 - 10240 - kv_number * sizeof(pair<ull, int>));
    for (ull i = 0; i < kv_number; i++) {
        index[i] = { curr->key, offset };
        offset += strlen(curr->value.c_str()) + 1;
        strcpy(data + index[i].second, curr->value.c_str());
        curr = curr->forward[0];
    }
}

SSTable::SSTable(const string &_dir, const string& sst_filename, int _level)
{
    level = _level;
    dir = _dir + "/" + "level-" + to_string(level) + "/" + sst_filename;
    ifstream file(dir, ios::binary | ios::in);
    file.read((char*)(&timestamp), sizeof(ull));
    file.read((char*)(&kv_number), sizeof(ull));
    file.read((char*)(&min_key), sizeof(ull));
    file.read((char*)(&max_key), sizeof(ull));
    file.read((char*)(bloom_filter), 10240 * sizeof(char));
    index = new pair<ull, int>[kv_number];
    file.read((char*)(index), sizeof(pair<ull, int>) * kv_number);
    data = NULL;
}

void SSTable::write()
{
    ofstream file(dir, ios::binary | ios::out);
    if(!file.is_open()){
        //create the directory
        string cmd = "mkdir " + dir.substr(0, dir.find_last_of('/'));
        system(cmd.c_str());
        file.open(dir, ios::binary | ios::out);
    }
    file.write((char*)this, 4*8+10240);
    file.write((char*)index, kv_number * sizeof(pair<ull, int>));
    file.write((char*)data, 0x200000 - 4 * 8 - 10240 - kv_number * sizeof(pair<ull, int>));
    file.close();
    delete[] data;
}

//search the key in the SSTable
pair<ull, int>* SSTable::search(ull key)
{
    if (key < min_key || key > max_key || !contains(bloom_filter, &key))
        return nullptr;
    ull left = 0, right = kv_number - 1;
    while (left < right) {
        ull mid = (left + right) / 2;
        if (index[mid].first < key)
            left = mid + 1;
        else
            right = mid;
    }
    if((index + left)->first == key)
        return index + left;
    return nullptr;
}

string SSTable::get_value(pair<ull, int> *index)
{
    string res;
    //read data from .sst file
    if (index) {
        ifstream file(dir, ios::binary | ios::in);
        int offset = index->second;
        file.seekg(4 * 8 + 10240 + kv_number * sizeof(pair<ull, int>) + offset);
        char* str = new char[0x200000 - 4 * 8 - 10240 - kv_number * sizeof(pair<ull, int>)];
        char* p = str;
        int len = 0;
        while(file.get(*p)){
            if(*p == 0)
                break;
            len++;
            p++;
        }
        char* value = new char[len + 1];
        strncpy(value, str, len + 1);
        res = value;
        delete[] str;
        delete[] value;
        file.close();
        return res;
    }
    return nullptr;
}

vector<pair<ull, string>>* SSTable::getAllData()
{
    vector<pair<ull, string>>* res = new vector<pair<ull, string>>();
    ifstream file(dir, ios::binary | ios::in);
    int offset = 0;
    for (ull i = 0; i < kv_number; i++) {
        string value = get_value(index + i);
        res->push_back({ index[i].first, value });
    }
    return res;
}

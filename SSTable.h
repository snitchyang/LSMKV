#pragma once
#include<vector>
#include<string>
#include<fstream>
#include "Skiplist.cpp"
typedef unsigned long long ull;

using namespace std;

class SSTable
{
public:
	ull timestamp;
	ull kv_number;
	ull min_key;
	ull max_key;
	char bloom_filter[10240];
	pair<ull, int>* index;
	char* data;
	int level;
	int id;
	string dir;

	SSTable();
	SSTable(const Skiplist& skiplist, ull _timestamp, const string& _dir, int level);
	SSTable(const string& kv_dir, const string& sst_filename, int _level);
	void write();
	pair<ull, int>* search(ull key);
	string get_value(pair<ull, int>* index);
	vector<pair<ull, string>>* getAllData();
};


#pragma once

#include "Skiplist.cpp"
#include "kvstore_api.h"
#include "SSTable.h"
#include "Compaction.cpp"
#include <algorithm>

class KVStore : public KVStoreAPI {
	// You can add your implementation here
public:
	Skiplist* MemTable;
	vector<vector<SSTable*>> SSTableList;
	int timestamp;
	int id;
	std::string directory;

public:
	KVStore(const std::string &dir);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

	void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string> > &list) override;

public:
	void init();

	void compact(int level);

	vector<SSTable*> choose_files(int level);

	void check_compaction();
};

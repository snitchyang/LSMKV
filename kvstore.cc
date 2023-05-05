#include "kvstore.h"
#include <cstring>
#include <unistd.h>
#include <dirent.h>

int current_level = 0;
KVStore::KVStore(const std::string &dir) : KVStoreAPI(dir)
{
	MemTable = new Skiplist;
	directory = dir;
	timestamp = 0;
	init();
}

KVStore::~KVStore()
{
	// if(MemTable)
	// 	delete MemTable;
	if (MemTable->size)
	{
		SSTableList[0].push_back(new SSTable(*MemTable, timestamp, directory, 0));
		SSTableList[0].back()->write();
		timestamp++;
	}
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{
	if (SSTableList.size() == 0)
	{
		SSTableList.push_back(vector<SSTable *>());
	}
	if (MemTable->total_length + s.length() > 2 * 1024 * 1024 - 10240 - 8 * 4 - sizeof(pair<uint64_t, string>) * MemTable->size - 1000)
	{
		// flush to disk
		SSTableList[0].push_back(new SSTable(*MemTable, timestamp, directory, 0));
		SSTableList[0].back()->write();
		timestamp++;
		delete MemTable;
		MemTable = new Skiplist;
	}
	MemTable->add(key, s);
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
	std::string res;

	// search in MemTable
	if (MemTable->search(key, res))
	{
		if (res == "~DELETED~")
			return "";
		return res;
	}

	// search in SSTable from the newest to the oldest
	int total_level = SSTableList.size();
	ull temp_timestamp = 0;
	for (int level = 0; level < total_level; level++)
	{
		int size = SSTableList[level].size();
		for (int i = size - 1; i >= 0; i--)
		{
			SSTable *sst = SSTableList[level][i];
			pair<ull, int> *p = sst->search(key);
			if (p != NULL && sst->timestamp >= temp_timestamp)
			{
				res = SSTableList[level][i]->get_value(p);
				temp_timestamp = sst->timestamp;
			}
		}
	}
	if (res == "~DELETED~")
		return "";
	else
	{
		return res;
	}
	return "";
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	std::string res = get(key);
	if (res == "")
		return false;
	put(key, "~DELETED~");
	return true;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
	Skiplist *tmp = MemTable;
	MemTable = new Skiplist;
	delete tmp;
}

/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 */
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list)
{
}

// read all the sstable files in the directory
void KVStore::init()
{
	// get the level number by reading the directory in C++11
	int level = 0;
	DIR *dir = NULL;
	struct dirent *ent;
	while (true)
	{
		std::string path = directory + "/level-" + std::to_string(level);
		if (access(path.c_str(), F_OK) == 0)
		{
			level++;
		}
		else
		{
			break;
		}
	}
	for (int i = 0; i < level; i++)
	{
		SSTableList.push_back(vector<SSTable *>());
		if ((dir = opendir((directory + "/" + "level-" + to_string(i)).c_str())) != NULL)
		{
			while ((ent = readdir(dir)) != NULL)
			{
				// if the filename includes .sst
				if (strstr(ent->d_name, ".sst") != NULL)
				{
					SSTableList[i].push_back(new SSTable(directory, ent->d_name, i));
				}
			}
			closedir(dir);
		}
	}
}

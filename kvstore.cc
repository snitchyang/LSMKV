#include "kvstore.h"
#include <cstring>
#include <unistd.h>
#include <dirent.h>

int current_level = 0;
KVStore::KVStore(const std::string &dir) : KVStoreAPI(dir)
{
	MemTable = new Skiplist;
	directory = dir;
	timestamp = -1;
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
		check_compaction();
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
		check_compaction();
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
		int temp_timestamp;
		SSTableList.push_back(vector<SSTable *>());
		if ((dir = opendir((directory + "/" + "level-" + to_string(i)).c_str())) != NULL)
		{
			while ((ent = readdir(dir)) != NULL)
			{
				// if the filename includes .sst
				if (strstr(ent->d_name, ".sst") != NULL)
				{
					SSTableList[i].push_back(new SSTable(directory, ent->d_name, i));
					char *p = strtok(ent->d_name, ".");
					temp_timestamp = atoi(p);
					SSTableList[i].back()->timestamp = temp_timestamp;
					if (temp_timestamp > timestamp)
						timestamp = temp_timestamp;
				}
			}
			closedir(dir);
		}
	}
	timestamp++;
}

void KVStore::compact(int level)
{
	vector<KVWithTimestamp *> res;
	// choose files to compact
	vector<SSTable *> files_to_compact = choose_files(level);
	res = compactToVector(files_to_compact, level);
	// find max timestamp
	ull max_timestamp = 0;
	for (auto p : res)
	{
		if (p->timestamp > max_timestamp)
			max_timestamp = p->timestamp;
	}

	// write to disk
	Skiplist *skiplist = new Skiplist();
	for (int i = 0; i < res.size(); i++)
	{
		if (skiplist->total_length + res[i]->value.length() > 2 * 1024 * 1024 - 10240 - 8 * 4 - sizeof(pair<uint64_t, string>) * skiplist->size - 1000)
		{
			// flush to disk
			SSTable *sst = new SSTable(*skiplist, max_timestamp, this->directory, level + 1);
			sst->write();
			if (SSTableList.size() <= level + 1)
			{
				SSTableList.push_back(vector<SSTable *>());
			}
			SSTableList[level + 1].push_back(sst);
			delete skiplist;
			skiplist = new Skiplist();
		}
		skiplist->add(res[i]->key, res[i]->value);
	}
	SSTable *sst = new SSTable(*skiplist, max_timestamp, this->directory, level + 1);
	sst->write();
	if (SSTableList.size() <= level + 1)
	{
		SSTableList.push_back(vector<SSTable *>());
	}
	SSTableList[level + 1].push_back(sst);
	delete skiplist;
}

vector<SSTable *> KVStore::choose_files(int level)
{
	vector<SSTable *> res;
	if (level == 0)
	{
		// add all the files in level0 to be compacted
		res = SSTableList[0];
		SSTableList[0].clear();
	}
	else
	{
		// sort SSTableList[level] by timestamp
		sort(SSTableList[level].begin(), SSTableList[level].end(), [](SSTable *a, SSTable *b)
			 { return a->timestamp < b->timestamp; });
		// choose the files with the smallest timestamp
		int current_size = SSTableList[level].size();
		for (int i = 0; i < current_size - pow(2, level + 1); i++)
		{
			res.push_back(SSTableList[level][0]);
			SSTableList[level].erase(SSTableList[level].begin());
		}
	}
	// get the min and max key of the SSTables
	ull min_key = INT64_MAX, max_key = 0;
	for (auto sst : res)
	{
		if (sst->min_key < min_key)
			min_key = sst->min_key;
		if (sst->max_key > max_key)
			max_key = sst->max_key;
	}
	if (SSTableList.size() <= level + 1)
		SSTableList.push_back(vector<SSTable *>());
	// choose files with overlapping keys in level + 1
	for (auto sst : SSTableList[level + 1])
	{
		if (sst->min_key <= max_key && sst->max_key >= min_key)
			res.push_back(sst);
	}
	return res;
}

void KVStore::check_compaction()
{
	// int level_number = SSTableList.size();
	// for (int i = 0; i < level_number; i++)
	// {
	// 	if (SSTableList[i].size() > pow(2, i + 1))
	// 	{
	// 		compact(i);
	// 	}
	// 	level_number = SSTableList.size();
	// }
}

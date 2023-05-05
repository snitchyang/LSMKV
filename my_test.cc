#include <iostream>
#include <cstdint>
#include <string>
#include <cstring>
#include "kvstore.h"
#include "SSTable.h"
#include "Compaction.h"

int main(){
    vector<vector<KVWithTimestamp*>> data;
    vector<KVWithTimestamp*> v1;
    v1.push_back(new KVWithTimestamp(1, "1", 2));
    v1.push_back(new KVWithTimestamp(2, "2", 2));
    v1.push_back(new KVWithTimestamp(3, "3", 2));
    v1.push_back(new KVWithTimestamp(4, "4", 2));
    v1.push_back(new KVWithTimestamp(5, "5", 2));
    v1.push_back(new KVWithTimestamp(6, "6", 2));
    vector<KVWithTimestamp*> v2;
    v2.push_back(new KVWithTimestamp(3, "33", 1));
    v2.push_back(new KVWithTimestamp(4, "44", 1));
    v2.push_back(new KVWithTimestamp(5, "55", 1));
    v2.push_back(new KVWithTimestamp(6, "66", 1));
    v2.push_back(new KVWithTimestamp(7, "77", 1));
    v2.push_back(new KVWithTimestamp(8, "88", 1));
    vector<KVWithTimestamp*> v3;
    v3.push_back(new KVWithTimestamp(5, "555", 3));
    v3.push_back(new KVWithTimestamp(6, "666", 3));
    v3.push_back(new KVWithTimestamp(7, "777", 3));
    v3.push_back(new KVWithTimestamp(8, "888", 3));
    v3.push_back(new KVWithTimestamp(9, "999", 3));
    v3.push_back(new KVWithTimestamp(10, "101010", 3));
    data.push_back(v1);
    data.push_back(v2);
    data.push_back(v3);
    vector<KVWithTimestamp*> res = merge(data);
    for(auto p : res){
        cout << p->key << " " << p->value << " " << p->timestamp << endl;
    }
    
}
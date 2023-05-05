#pragma once
#include <iostream>
#include "SSTable.h"

struct KVWithTimestamp{
    ull key;
    string value;
    ull timestamp;
    //constructor
    KVWithTimestamp(ull key, string value, ull timestamp):key(key), value(value),timestamp(timestamp){}
};

void compact(const vector<SSTable*>& SSTableList, int level){
    //store data in SSTablelist in a vector
    vector<vector<KVWithTimestamp*>> data;
    for(auto sst : SSTableList){
        vector<pair<ull, string>>* v = sst->getAllData();
        data.push_back(vector<KVWithTimestamp*>());
        for(auto p : *v){
            data.back().push_back(new KVWithTimestamp(p.first, p.second, sst->timestamp));
        }
        delete v;
    }
}

vector<KVWithTimestamp*> merge(vector<vector<KVWithTimestamp*>> data){
    if(data.size() == 1)
        return data[0];
    int mid = data.size() / 2;
    vector<vector<KVWithTimestamp*>> left(data.begin(), data.begin() + mid);
    vector<vector<KVWithTimestamp*>> right(data.begin() + mid, data.end());
    vector<KVWithTimestamp*> left_res = merge(left);
    vector<KVWithTimestamp*> right_res = merge(right);
    //merge left and right result
    int i = 0;
    int j = 0;
    vector<KVWithTimestamp*> res;
    while(i < left_res.size() && j < right_res.size()){
        if(left_res[i]->key < right_res[j]->key){
            res.push_back(left_res[i]);
            i++;
        }
        else if(left_res[i]->key > right_res[j]->key){
            res.push_back(right_res[j]);
            j++;
        }
        else{
            //same, compare timestamp
            if(left_res[i]->timestamp > right_res[j]->timestamp)
                res.push_back(left_res[i]);
            else
                res.push_back(right_res[j]);
            i++;
            j++;
        }
    }
    while(i < left_res.size()){
        res.push_back(left_res[i]);
        i++;
    }
    while(j < right_res.size()){
        res.push_back(right_res[j]);
        j++;
    }
    return res;
}


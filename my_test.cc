#include <iostream>
#include <cstdint>
#include <string>
#include <cstring>
#include "kvstore.h"
#include "SSTable.h"

int main(){
    KVStore store("./data");
    for(int i = 0; i < 10000; i++){
        store.put(i, std::string(i + 1, 'a'));
    }
    // for(int i = 0; i < 10000; i++){
    //     std::string res = store.get(i);
    //     if(res != std::string(i + 1, 'a')){
    //         std::cout << "error on " << i << std::endl;
    //         return 0;
    //     }
    // }
    cout << store.get(178) << endl;
}
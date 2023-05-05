#if !defined(SKIPLIST_H)
#define SKIPLIST_H


#include <iostream>
#include <vector>
#include <random>
typedef unsigned long long ull;

using namespace std;

constexpr int MAX_LEVEL = 32;
constexpr double P_FACTOR = 0.25;

struct SkiplistNode {
    ull key;
    string value;
    vector<SkiplistNode*> forward;
    SkiplistNode(int _key, string _value = "", int _maxLevel = MAX_LEVEL) : key(_key), value(_value), forward(_maxLevel, nullptr) {

    }
};

class Skiplist {
public:
    SkiplistNode* head;
    int level;
    ull size;
    ull total_length;
    mt19937 gen{ random_device{}() };
    uniform_real_distribution<double> dis;

public:
    Skiplist() : head(new SkiplistNode(-1)), level(0), dis(0, 1) {
        size = 0;
        total_length = 0;
    }

    bool search(ull target, string &value) {
        SkiplistNode* curr = this->head;
        for (int i = level - 1; i >= 0; i--) {
            while (curr->forward[i] && curr->forward[i]->key < target) {
                curr = curr->forward[i];
            }
        } 
        curr = curr->forward[0];
        if (curr && curr->key == target) {
            value = curr->value;
            return true;
        }
        return false;
    }

    void add(ull num, string value) {
        total_length += (value.length() + 1);
        vector<SkiplistNode*> update(MAX_LEVEL, head);
        SkiplistNode* curr = this->head;
        for (int i = level - 1; i >= 0; i--) {
            while (curr->forward[i] && curr->forward[i]->key < num) {
                curr = curr->forward[i];
            }
            update[i] = curr;
        }
        int lv = randomLevel();
        level = max(level, lv);
        SkiplistNode* newNode = new SkiplistNode(num, value, lv);
        for (int i = 0; i < lv; i++) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        size++;
    }

    bool erase(ull num) {
        vector<SkiplistNode*> update(MAX_LEVEL, nullptr);
        SkiplistNode* curr = this->head;
        for (int i = level - 1; i >= 0; i--) {
            while (curr->forward[i] && curr->forward[i]->key < num) {
                curr = curr->forward[i];
            }
            update[i] = curr;
        }
        curr = curr->forward[0];
        if (!curr || curr->key != num) {
            return false;
        }
        total_length -= (curr->value.length() + 1);
        for (int i = 0; i < level; i++) {
            if (update[i]->forward[i] != curr) {
                break;
            }
            update[i]->forward[i] = curr->forward[i];
        }
        delete curr;
        while (level > 1 && head->forward[level - 1] == nullptr) {
            level--;
        }
        size--;
        return true;
    }

    int randomLevel() {
        int lv = 1;
        while (dis(gen) < P_FACTOR && lv < MAX_LEVEL) {
            lv++;
        }
        return lv;
    }

    void clear(){
        SkiplistNode* curr = head->forward[0];
        while (curr) {
            SkiplistNode* tmp = curr;
            curr = curr->forward[0];
            delete tmp;
        }
        for (int i = 0; i < MAX_LEVEL; i++) {
            head->forward[i] = nullptr;
        }
        level = 0;
        size = 0;
        total_length = 0;
    }

    //display the skiplist
    void display(){
        for (int i = level - 1; i >= 0; i--) {
            if(i != 0) continue;
            SkiplistNode* curr = head->forward[i];
            cout << "level " << i << ": ";
            while (curr) {
                cout << curr->key << " ";
                curr = curr->forward[i];
            }
            cout << endl;
        }
    }

    ~Skiplist() {
        clear();
        delete head;
    }
};

#endif 

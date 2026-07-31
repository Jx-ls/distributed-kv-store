#pragma once

#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class Storage {
public:
    void put(const string& key, const string& value);
    bool get(const string& key, string& value);
    vector<string> get_all_keys() const;
    void clear();

private:
    unordered_map<string, string> kv;
};
#pragma once

#include <string>
#include <unordered_map>
using namespace std;

class Storage {
public:
    void put(const string& key, const string& value);
    bool get(const string& key, string& value);

private:
    unordered_map<string, string> kv;
};
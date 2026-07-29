#include "../include/storage.h"

using namespace std;

void Storage::put(const string& key, const string& value) {
    kv[key] = value;
}

bool Storage::get(const string& key, string& value) {
    auto it = kv.find(key);
    if (it == kv.end())
        return false;
    value = it -> second;
    return true;
}

vector<string> Storage::get_all_keys() const {
    vector<string> keys;
    keys.reserve(kv.size());
    for (const auto& [k, v] : kv) {
        keys.push_back(k);
    }
    return keys;
}
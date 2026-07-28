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
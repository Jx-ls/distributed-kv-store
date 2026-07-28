#include "../include/storage.h"
#include "../include/log.h"

#include <iostream>

int main()
{
    Storage store;

    store.put("x","10");
    store.put("y","20");

    std::string value;

    if(store.get("x",value))
        std::cout << value << '\n';

    Log log;

    log.append({1,"a","100"});
    log.append({1,"b","200"});

    std::cout << log.size() << '\n';
}
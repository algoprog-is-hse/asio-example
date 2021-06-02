#include "lib.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

int not_random() { return 3; }

void process_client(std::iostream &stream) {
    nlohmann::json obj;
    stream >> obj;
    if (obj.is_object() && obj.contains("count")) {
        char str[] = "hello, ";
        int n = obj["count"].get<int>();
        for (int i = 0; i < n; ++i)
            stream << str;
    }
    if (obj.is_object() && obj.contains("text"))
        stream << obj["text"] << "\n";
}
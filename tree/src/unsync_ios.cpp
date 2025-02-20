#include <iostream>

void unsync_ios() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
}
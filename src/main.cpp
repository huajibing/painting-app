#include "core/application.hpp"
#include <iostream>

int main() {
    Application app;
    
    if (!app.init()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.run();
    return 0;
}
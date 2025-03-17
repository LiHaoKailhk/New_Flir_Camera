// ErrorHandler.h
#pragma once
#include <exception>
#include <string>

class ErrorHandler {
public:
    static void HandleException(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
};
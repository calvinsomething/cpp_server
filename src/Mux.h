#pragma once

#include <iostream>

#include "Request.h"

void mux(Request& request) {
    std::cout << "MUX:::" << request.method << std::endl;
}
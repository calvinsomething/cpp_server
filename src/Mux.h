#pragma once

#include <iostream>

#include "Request.h"
#include "ResponseWriter.h"

void mux(Request request, ResponseWriter response_writer)
{
    std::cout << "MUX:::" << request.method << std::endl;
}
#pragma once

#include "Request.h"
#include "ResponseWriter.h"

void mux(Request request, ResponseWriter response_writer);

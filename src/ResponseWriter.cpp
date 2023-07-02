#include "ResponseWriter.h"

#include <unistd.h>


void ResponseWriter::write_text(std::string text)
{
    write(fd, static_cast<const void*>(text.c_str()), text.size());
}

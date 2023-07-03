#include "ResponseWriter.h"

#include <unistd.h>
#include <sys/socket.h>


void ResponseWriter::write_text(std::string text)
{
    send(fd, static_cast<const void*>(text.c_str()), text.size(), 0);
}

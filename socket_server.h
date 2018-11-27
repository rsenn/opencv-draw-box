#include "socket_header.h"
#include "image.h"
#include "cJSON.h"


std::string receive_message(int port, int timeout, int quality);

std::vector<uchar> receive_frame(int port, int timeout, int quality);
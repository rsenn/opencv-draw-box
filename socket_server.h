#include "socket_header.h"
#include "image.h"
#include "cJSON.h"


std::string receive_message(int port, int timeout, int quality);

std::vector<unsigned char> receive_frame(int port, int timeout, int quality);
int receive_frame(std::vector<unsigned char> &frame, int port, int timeout, int quality);
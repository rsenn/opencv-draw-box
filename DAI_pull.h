#ifndef DAI_PUSH
#define DAI_PUSH

#include "image.h" // person_box
#include <iostream>
#include <vector> // std::vector

void iot_init();
void iot_talk_receive(std::vector<person_box> &boxes);
void iot_receive(std::vector<unsigned char> &frame, std::vector<person_box> &boxes);

#endif
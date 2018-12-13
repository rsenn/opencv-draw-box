#ifndef FUSION_H
#define FUSION_H

#include "image.h"

void convert_coordinate(std::vector<cv::Point2f> &box_position, std::vector<cv::Point2f> &beacon_position);

#endif // FUSION_H
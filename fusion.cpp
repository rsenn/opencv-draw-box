#include "fusion.h"

#include <iostream>

// beacon position on the picture
static const cv::Point2f box_map_rt = cv::Point2f(800, 100); // box right top
static const cv::Point2f box_map_rb = cv::Point2f(800, 600); // box right bottom
static const cv::Point2f box_map_lt = cv::Point2f(100, 100); // box left top
static const cv::Point2f box_map_lb = cv::Point2f(100, 600); // box left bottom

// beacon position in the java code
static const cv::Point2f beacon_map_rt = cv::Point2f(540, 100); // beacon right top
static const cv::Point2f beacon_map_rb = cv::Point2f(540, 1100); // beacon right bottom
static const cv::Point2f beacon_map_lt = cv::Point2f(10, 100); // beacon left top
static const cv::Point2f beacon_map_lb = cv::Point2f(10, 1100); // beacon left bottom

// box -> beacon transform matrix
static cv::Mat transform_matrix;

// convert box position to beacon position
// @box_position: std::vector of cv::Point2f for storing box center (x, y)
// @beacon_position: std::vector of cv::Point2f for storing beacon position (x, y) on the picture
void convert_coordinate(std::vector<cv::Point2f> &box_position, std::vector<cv::Point2f> &beacon_position)
{
    // box mapping points
    std::vector<cv::Point2f> box_map_points = { box_map_rt, box_map_rb, box_map_lt, box_map_lb };
 
    // beacon mapping points
	std::vector<cv::Point2f> beacon_map_points = { beacon_map_rt, beacon_map_rb, beacon_map_lt, beacon_map_lb };

    // check whether transform_matrix is created or not
    if(transform_matrix.empty())
    {
        // create transform_matrix
        transform_matrix = cv::getPerspectiveTransform(box_map_points, beacon_map_points);
    }

    // transform input box_position to output beacon_position
    cv::perspectiveTransform(box_position, beacon_position, transform_matrix);

    //std::cout<<src_points.size()<<std::endl;
    //std::cout<<dst_points.size()<<std::endl;

    //std::cout<<transform_matrix.size()<<std::endl;
    //std::cout<<transform_matrix<<std::endl;

    //std::cout<<((float *)transform_matrix.data)[0]<<std::endl;

    //std::cout<<beacon_position[0].x<<", "<<beacon_position[0].y<<std::endl;
}
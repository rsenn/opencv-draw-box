#include "image.h"
#include "mjpeg_streaming.h"
#include "socket_server.h"
#include "DAI_pull.h"
#include "fusion.h"

#include <iostream>
#include <vector>
#include <map>

#include <fstream> // read tag file

//static void * cap;
static cv::Mat m;
static image im;
static image **alphabet;
static std::vector<unsigned char> frame;
static std::vector<person_box> boxes;

static std::map<std::string, image> tag_pics;

static std::map<int, std::vector<unsigned char>> frame_buffer;
static std::vector<int> frame_buffer_remove;

void load_tag_pic()
{
    tag_pics.clear();

    std::ifstream in("tag_pic/tag_pic.txt");

    if(!in)
    {
        std::cout<<"Cannot open input file."<<std::endl;
        exit(EXIT_FAILURE);
    }

    std::string str, tag_str;

    while (std::getline(in, str))
    {
        // output the line
        tag_str = "tag_pic/" + str;
        str = str.substr(0, str.size() - 4); // remove .jpg
        std::cout<<str<<std::endl;
        std::cout<<tag_str<<std::endl;
        tag_pics.insert(std::pair<std::string, image>(str, resize_image(load_image_color((char *)tag_str.c_str(), 100, 100), 100, 100)));

        /*cv::Mat m = image_to_mat(tag_pics);
        imshow(str, m);
        waitKey(1);*/
    }

    in.close();
}

void draw_tag_pic(image im, std::string tag_name, int &all_tag_count, int &legal_tag_count)
{
    int tag_pic_width = 220;
    int is_legal_tag = 0;

    int dx = im.w - tag_pic_width + 60;
    int dy;

    if (tag_name != "unknown")
    {
        legal_tag_count++;
        dy = 40 + (int)100 * (legal_tag_count - 1); // legal tag position
        is_legal_tag = 1;
    }
    else
    {
        dy = 40 + (int)160 * (all_tag_count - legal_tag_count); // unknown tag position
    }

    all_tag_count++;

    dy += (is_legal_tag) ? 30 : (int)im.h / 2;

    embed_image(tag_pics[tag_name], im, dx, dy);

    // draw label on tag
    float rgb[3];
    if(is_legal_tag)
    {
        image label = get_label(alphabet, (char *)tag_name.c_str(), (im.h*.03)/10);
        
        rgb[0] = 0.0;
        rgb[1] = 1.0;
        rgb[2] = 0.0;

        //draw_label(im, dy, dx - 40, label, rgb);
        draw_label(im, dy, dx, label, rgb);
        free_image(label);
    }
    else
    {
        image label = get_label(alphabet, (char *)tag_name.c_str(), (im.h*.03)/16);

        rgb[0] = 1.0;
        rgb[1] = 0.0;
        rgb[2] = 0.0;

        draw_label(im, dy, dx, label, rgb);
        free_image(label);
    }

}

int main()
{
    /*float box_x = (575 + 681) / 2;
    float box_y = (177 + 254) / 2;
    std::cout<<box_x<<", "<<box_y<<std::endl;
    std::vector<cv::Point2f> box_position = { cv::Point2f(box_x, box_y) };
    std::vector<cv::Point2f> beacon_position;
    convert_coordinate(box_position, beacon_position);

    std::cout<<beacon_position[0].x<<", "<<beacon_position[0].y<<std::endl;*/


    load_tag_pic();
    alphabet = load_alphabet();
    int red = 0, green = 0, blue = 255; // box color
    float rgb[3] = {red, green, blue};

    int width = 3; // box line width
    int alphabet_size = 3;

    int left = 50; // x1
    int top = 50; // y1
    int right = 200; // x2
    int bot = 300; // y2
    std::string name = "person_name";

    int legal_tag_count = 0;
    int all_tag_count = 0;
    int frame_stamp = 0;

    //cv::VideoCapture *cap = new cv::VideoCapture("time_counter.flv");
    //cv::VideoCapture *cap = new cv::VideoCapture("test.MTS");

    iot_init();

    frame_buffer.clear();

    while(1)
    {
        frame_stamp = receive_frame(frame, 8091, 200, 95);

        if(frame_stamp)
        {
            std::cout<<"frame_stamp : "<<frame_stamp<<std::endl;

            frame_buffer.insert(std::pair<int, std::vector<unsigned char>>(frame_stamp, frame));
        }
            

        /*
        
        /*if(!frame.empty())
        {
            m = cv::imdecode(frame, 1);
            send_mjpeg(m, 8090, 200, 95);
        }*/

        boxes.clear();
        iot_talk_receive(boxes);

        //if(!frame.empty())
        if(boxes.size())
        {
            int current_frame_stamp = boxes[0].frame_stamp;

            if(frame_buffer.find(current_frame_stamp) != frame_buffer.end())
            {
                m = cv::imdecode(frame_buffer[current_frame_stamp], 1);
                im = mat_to_image(m);
            }
            else
            {
                continue;
            }

            frame_buffer_remove.clear();
            for(auto &it : frame_buffer)
            {
                if(it.first < current_frame_stamp)
                {
                    frame_buffer_remove.push_back(it.first);
                }
            }

            char frame_stamp_label_str[4096] = {0};
            sprintf(frame_stamp_label_str, "%d - %d", current_frame_stamp, frame_buffer.size());
            image frame_stamp_label = get_label(alphabet, frame_stamp_label_str, (im.h*.03));
            draw_label(im, 0, 0, frame_stamp_label, rgb);

            for(auto &it : frame_buffer_remove)
            {
                frame_buffer.erase(it);
            }

            for(int i = 0; i < boxes.size(); i++)
            {
                printf("%d = %d\n", boxes[i].frame_stamp, boxes[i].id);

                left = boxes[i].x1;
                top = boxes[i].y1;
                right = boxes[i].x2;
                bot = boxes[i].y2;

                boxes[i].frame_stamp;

                printf("person id : %d\nx1: %d, y1: %d\nx2: %d, y2: %d\n", boxes[i].id, left, top, right, bot);

                width = im.h * .006;
                draw_box_width(im, left, top, right, bot, width, red, green, blue);

                char labelstr[4096] = {0};
                sprintf(labelstr, "%d", boxes[i].id);
                image label = get_label(alphabet, labelstr, (im.h*.03));
                draw_label(im, top + width, left, label, rgb);
            }

            m = image_to_mat(im);
            send_mjpeg(m, 8090, 200, 95);
        }
    }
    

    /*while(1)
    {
        boxes.clear();
        iot_talk_receive(boxes);
        //std::cout<<boxes.size()<<std::endl;    

        if(boxes.size() > 0)
        {
            *cap >> m;
            im = mat_to_image(m);   
        }
        
        for(int i = 0; i < boxes.size(); i++)
        {
            printf("%d = %d\n", boxes[i].frame_stamp, boxes[i].id);

            left = boxes[i].x1;
            top = boxes[i].y1;
            right = boxes[i].x2;
            bot = boxes[i].y2;

            printf("person id : %d\nx1: %d, y1: %d\nx2: %d, y2: %d\n", boxes[i].id, left, top, right, bot);

            width = im.h * .006;
            draw_box_width(im, left, top, right, bot, width, red, green, blue);

            char labelstr[4096] = {0};
            sprintf(labelstr, "%d = %d", boxes[i].frame_stamp, boxes[i].id);
            image label = get_label(alphabet, labelstr, (im.h*.03));
            draw_label(im, top + width, left, label, rgb);
        }

        if(!m.empty())
        {
            m = image_to_mat(im);
            send_mjpeg(m, 8090, 200, 95);
        }
        

        //cv::imshow("Receiver", m);
        //cv::waitKey(1);
    }*/

    /*while(1)
    {
        //iot_receive(frame, boxes);

        if(!frame.empty())
        {
            m = cv::imdecode(frame, 1);
            im = mat_to_image(m);
            
            width = im.h * .006;
            alphabet_size = im.h*.03;

            image label;

            legal_tag_count = 0;
            all_tag_count = 0;

            for(int i = 0; i < boxes.size(); i++)
            {
                if(boxes[i].name == "unknown")
                {
                    red = 255; green = 0; blue = 0;
                }
                else
                {
                    red = 0; green = 255; blue = 0;
                }

                rgb[0] = red; rgb[1] = green; rgb[2] = blue; // setting color

                left = boxes[i].x1;
                top = boxes[i].y1;
                right = boxes[i].x2;
                bot = boxes[i].y2;
                draw_box_width(im, left, top, right, bot, width, red, green, blue);
                image label = get_label(alphabet, (char *)boxes[i].name.c_str(), alphabet_size);
                draw_label(im, top + width + alphabet_size, left, label, rgb);

                if(tag_pics.find(boxes[i].name) != tag_pics.end())
                {
                    // draw right board for picture in tag_pic folder
                    draw_tag_pic(im, boxes[i].name, all_tag_count, legal_tag_count);
                    //std::cout<<"find tag picture, "<<all_tag_count<<", "<<legal_tag_count<<std::endl;
                }
            }

            //draw_box_width(im, left, top, right, bot, width, red, green, blue);
            
            //image label = get_label(alphabet, (char *)name.c_str(), alphabet_size);
            //draw_label(im, top + width + alphabet_size, left, label, rgb);

            m = image_to_mat(im);

            //send_mjpeg(m, 8090, 200, 95);

            cv::imshow("Receiver", m);
            cv::waitKey(1);
        }
    }*/

    return 0;
}

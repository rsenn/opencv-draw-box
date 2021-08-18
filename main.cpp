#include "image.h"
#include "mjpeg_streaming.h"
#include "socket_server.h"
#include <iostream>
#include <vector>
#include <map>
#include <Python.h>

#include <fstream>
#include <filesystem>

static void * cap;
static cv::Mat m;
static image im;
static image **alphabet;
static std::vector<uchar> frame;
static std::vector<person_box> boxes;

static std::map<std::string, image> tag_pics;

static PyObject *pModule, *pDict, *pFunc, *pFrameList, *pBoxDict, *pBoxList, *pTuple;

#define CHECK_PYTHON_NULL(p) \
    if (NULL == (p)) {\
        PyErr_Print();\
        exit(EXIT_FAILURE);\
    }

void iot_init()
{
    std::wstring pypath = Py_GetPath();

    pypath += ':';
    pypath += (std::filesystem::current_path() / std::filesystem::path("iottalk")).wstring();

    Py_Initialize();

    PySys_SetPath(pypath.c_str());

    std::wcout << "PYTHONPATH=" << pypath << std::endl;

    //setenv("PYTHONPATH", PYTHON_IMPORT_PATH, 1);

    pModule = PyImport_ImportModule("DAI_pull");
    CHECK_PYTHON_NULL(pModule)

    pDict = PyModule_GetDict(pModule);
    CHECK_PYTHON_NULL(pDict)

    pFunc = PyDict_GetItemString(pDict, "receive_frame_from_iottalk");
    CHECK_PYTHON_NULL(pFunc)
}

void iot_receive()
{
    /*PyObject *PyList  = PyList_New(outbuf.size());
    PyObject *ArgList = PyTuple_New(1);

    for(int i = 0; i < PyList_Size(PyList); i++)
    {
        PyList_SetItem(PyList, i, PyInt_FromLong((int)outbuf[i]));
    }

    PyTuple_SetItem(ArgList, 0, PyList);

    if(PyCallable_Check(pFunc))
    {
        PyObject_CallObject(pFunc, ArgList);
    }
    else
    {
        PyErr_Print();
    }*/

    frame.clear();
    boxes.clear();

    // *pModule, *pDict, *pFunc, *pFrameList, *pBoxDict, *pBoxList, *pTuple;

    if(PyCallable_Check(pFunc))
    {
        pTuple = PyObject_CallObject(pFunc, NULL);

        if(PyTuple_Check(pTuple))
        {
            pFrameList = PyTuple_GetItem(pTuple, 0);
            pBoxList = PyTuple_GetItem(pTuple, 1);

            //std::cout<<"pFrameList"<<std::endl;

            if(PyList_Check(pFrameList))
            {
                int pFrameListSize = PyList_Size(pFrameList);
                for(int i = 0; i < pFrameListSize; i++)
                {
                    PyObject *pFrameListItem = PyList_GetItem(pFrameList, i);

                    uchar num = (uchar)PyLong_AsLong(pFrameListItem);

                    frame.push_back(num);

                    Py_DECREF(pFrameListItem);
                }
            }

            //std::cout<<"pBoxList"<<std::endl;

            if(PyList_Check(pBoxList))
            {
                std::string name;
                int id;
                int x1, y1, x2, y2;
                int pBoxListSize = PyList_Size(pBoxList);

                for(int i = 0; i < pBoxListSize; i++)
                {
                    pBoxDict = PyList_GetItem(pBoxList, i);

                    //std::cout<<PyBytes_AsString(PyDict_GetItemString(pBoxDict, "name"))<<std::endl;

                    name = PyBytes_AsString(PyDict_GetItemString(pBoxDict, "name"));
                    id = PyLong_AsLong(PyDict_GetItemString(pBoxDict, "id"));
                    x1 = PyLong_AsLong(PyDict_GetItemString(pBoxDict, "x1"));
                    y1 = PyLong_AsLong(PyDict_GetItemString(pBoxDict, "y1"));
                    x2 = PyLong_AsLong(PyDict_GetItemString(pBoxDict, "x2"));
                    y2 = PyLong_AsLong(PyDict_GetItemString(pBoxDict, "y2"));

                    person_box b = {name, id, x1, y1, x2, y2};
                    boxes.push_back(b);

                    //uchar num = (uchar)PyLong_AsLong(pBoxListItem);

                    //frame.push_back(num);

                    //Py_DECREF(pBoxDict);
                }
            }
        }
    }
    else
    {
        PyErr_Print();
    }
}

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

    iot_init();

    while(1)
    {
        iot_receive();

        if(!frame.empty())
        {
            m = imdecode(frame, 1);
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

            send_mjpeg(m, 8090, 200, 95);

            //imshow("Receiver", m);
            //waitKey(1);
        }
    }

    /*while(1)
    {
        frame = receive_frame(8090, 200, 95);

        if(frame.empty())
        {

        }
        else
        {
            cv::Mat m = imdecode(frame, 1);
            imshow("Receiver", m);
            waitKey(1);
        }
        
    }*/

    /*char buffer[BUFFER_MAX];
    memset(buffer, '\0', sizeof(char) * BUFFER_MAX);

    strcpy(buffer, "test for buffer");

    cout<<strlen(buffer)<<endl;

    string message;
    message.clear();

    message += buffer;

    cout<<message.size()<<endl;*/


    //image im = load_image_color("predictions.jpg", 0, 0);

    //im = resize_image(im, 100, 100);

    //int w = im.w, h = im.h, c = im.c;
    //int im_size = h*w*c;

    //printf("w=%d, h=%d, c=%d, im_size=%d\n", w, h, c, im_size);

    /*for(int i=0; i<im_size; i++)
    {
        if(i % 100 == 0) cout<<endl;

        //cout<<(int)(im.data[i]*(255))<<"  ";
    }*/

    //show_image_cv(im, "predictions", 0);

    //int c =  show_image_cv(im, "predictions", 0);

    /*image in = load_image("predictions.jpg", 768, 576, 3);
    int c = show_image_cv(in, "Demo", 0);*/
    //cout<<c<<endl;

    //while(1);

    /*alphabet = load_alphabet(); // read all alphabet picture

    cap = open_video_stream("test.MTS", 0, 0, 0, 0); // open video

    int red = 0, green = 0, blue = 255; // box color
    float rgb[3] = {red, green, blue};

    int width = 3; // box line width
    int alphabet_size = 3;

    int left = 50; // x1
    int top = 50; // y1
    int right = 200; // x2
    int bot = 300; // y2

    while(1)
    {
        im = get_image_from_stream(cap);

        im = resize_image(im, 800, 600);

        int w = im.w, h = im.h, c = im.c;
        int im_size = h*w*c;

        printf("w=%d, h=%d, c=%d, im_size=%d\n", w, h, c, im_size);

        for(int i=0; i<im_size; i++)
        {
            if(i % 10 == 0) cout<<endl;

            cout<<im.data[i]<<"  "<<endl;
        }

        //printf("w=%d, h=%d, , c=%d, size=%d\n", im.w, im.h, im.c, (int)sizeof(im.data));

        width = im.h * .012;
        alphabet_size = im.h*.03;
        draw_box_width(im, 50, 50, 200, 300, width, red, green, blue);
        image label = get_label(alphabet, "person_name", alphabet_size);
        draw_label(im, top + width + alphabet_size, left, label, rgb);
        send_mjpeg(im, 8090, 200, 95);
        //int c = show_image_cv(im, "Demo", 1);
        //cout<<c<<endl;
    }*/

    return 0;
}

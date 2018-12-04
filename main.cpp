#include "image.h"
#include "mjpeg_streaming.h"
#include "socket_server.h"
#include <iostream>
#include <Python.h>

using namespace std;

static void * cap;
static cv::Mat m;
static image im;
static image **alphabet;
static std::vector<uchar> frame;
static std::vector<person_box> boxes;

static PyObject *pModule, *pDict, *pFunc, *pFrameList, *pBoxDict, *pBoxList, *pTuple;

#define CHECK_PYTHON_NULL(p) \
    if (NULL == (p)) {\
        PyErr_Print();\
        exit(EXIT_FAILURE);\
    }

void iot_init()
{
    Py_Initialize();

    //PySys_SetPath(PYTHON_IMPORT_PATH);

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

                    uchar num = (uchar)PyInt_AsLong(pFrameListItem);

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

                    //std::cout<<PyString_AsString(PyDict_GetItemString(pBoxDict, "name"))<<std::endl;

                    name = PyString_AsString(PyDict_GetItemString(pBoxDict, "name"));
                    id = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "id"));
                    x1 = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "x1"));
                    y1 = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "y1"));
                    x2 = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "x2"));
                    y2 = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "y2"));

                    person_box b = {name, id, x1, y1, x2, y2};
                    boxes.push_back(b);

                    //uchar num = (uchar)PyInt_AsLong(pBoxListItem);

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

int main()
{
    alphabet = load_alphabet();
    int red = 0, green = 0, blue = 255; // box color
    float rgb[3] = {red, green, blue};

    int width = 3; // box line width
    int alphabet_size = 3;

    int left = 50; // x1
    int top = 50; // y1
    int right = 200; // x2
    int bot = 300; // y2
    string name = "person_name";

    iot_init();

    while(1)
    {
        iot_receive();

        if(!frame.empty())
        {
            m = imdecode(frame, 1);
            im = mat_to_image(m);
            
            width = im.h * .012;
            alphabet_size = im.h*.03;

            image label;

            for(int i = 0; i < boxes.size(); i++)
            {
                left = boxes[i].x1;
                top = boxes[i].y1;
                right = boxes[i].x2;
                bot = boxes[i].y2;
                draw_box_width(im, left, top, right, bot, width, red, green, blue);
                image label = get_label(alphabet, (char *)boxes[i].name.c_str(), alphabet_size);
                draw_label(im, top + width + alphabet_size, left, label, rgb);
            }

            //draw_box_width(im, left, top, right, bot, width, red, green, blue);
            
            //image label = get_label(alphabet, (char *)name.c_str(), alphabet_size);
            //draw_label(im, top + width + alphabet_size, left, label, rgb);

            m = image_to_mat(im);

            imshow("Receiver", m);
            waitKey(1);
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
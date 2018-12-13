#include "DAI_pull.h"

#include <Python.h>

static PyObject *pModule, *pDict, *pFunc, *pFrameList, *pBoxDict, *pBoxList, *pTuple;

#define CHECK_PYTHON_NULL(p) \
    if (NULL == (p)) {\
        PyErr_Print();\
        exit(EXIT_FAILURE);\
    }


// initial python compiler and import DAI_pull.py module
void iot_init()
{
    Py_Initialize();

    // load module
    pModule = PyImport_ImportModule("DAI_pull");
    CHECK_PYTHON_NULL(pModule)

    // get all function as dictionary in module
    pDict = PyModule_GetDict(pModule);
    CHECK_PYTHON_NULL(pDict)

    // get function receive_frame_from_iottalk
    pFunc = PyDict_GetItemString(pDict, "receive_frame_from_iottalk");
    CHECK_PYTHON_NULL(pFunc)
}

void iot_talk_receive(std::vector<person_box> &boxes)
{
    // check whether the function can be called or not
    if(PyCallable_Check(pFunc))
    {
        // call function and get return (ret1, ret2) to pTuple
        pTuple = PyObject_CallObject(pFunc, NULL);

        // check whether the pTuple type is tuple or not
        if(PyTuple_Check(pTuple))
        {
            // get value from the pTuple pTuple[0], pTuple[1]
            pBoxList = PyTuple_GetItem(pTuple, 0);

            // check whether the pBoxList type is list or not (json style)
            if(PyList_Check(pBoxList))
            {
                // all class in the json
                //std::string name;
                int frame_stamp;
                int id;
                int x1, y1, x2, y2;
                int pBoxListSize = PyList_Size(pBoxList);

                // get all class value
                for(int i = 0; i < pBoxListSize; i++)
                {
                    pBoxDict = PyList_GetItem(pBoxList, i);

                    //std::cout<<PyString_AsString(PyDict_GetItemString(pBoxDict, "name"))<<std::endl;

                    frame_stamp = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "stamp"));
                    id = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "id"));
                    x1 = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "x1"));
                    y1 = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "y1"));
                    x2 = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "x2"));
                    y2 = PyInt_AsLong(PyDict_GetItemString(pBoxDict, "y2"));

                    person_box b = {frame_stamp, id, x1, y1, x2, y2};
                    boxes.push_back(b);

                    //unsigned char num = (unsigned char)PyInt_AsLong(pBoxListItem);

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

    //std::cout<<boxes.size()<<std::endl;
}

// pull data from iot talk
// @frame: 
// @boxes
/*void iot_receive(std::vector<unsigned char> &frame, std::vector<person_box> &boxes)
{
    frame.clear();
    boxes.clear();

    // check whether the function can be called or not
    if(PyCallable_Check(pFunc))
    {
        // call function and get return (ret1, ret2) to pTuple
        pTuple = PyObject_CallObject(pFunc, NULL);

        // check whether the pTuple type is tuple or not
        if(PyTuple_Check(pTuple))
        {
            // get value from the pTuple pTuple[0], pTuple[1]
            pFrameList = PyTuple_GetItem(pTuple, 0);
            pBoxList = PyTuple_GetItem(pTuple, 1);

            //std::cout<<"pFrameList"<<std::endl;

            // check whether the pFrameList type is list or not
            if(PyList_Check(pFrameList))
            {
                int pFrameListSize = PyList_Size(pFrameList);
                for(int i = 0; i < pFrameListSize; i++)
                {
                    PyObject *pFrameListItem = PyList_GetItem(pFrameList, i);

                    unsigned char num = (unsigned char)PyInt_AsLong(pFrameListItem);

                    frame.push_back(num);

                    Py_DECREF(pFrameListItem);
                }
            }

            //std::cout<<"pBoxList"<<std::endl;

            // check whether the pBoxList type is list or not (json style)
            if(PyList_Check(pBoxList))
            {
                // all class in the json
                std::string name;
                int id;
                int x1, y1, x2, y2;
                int pBoxListSize = PyList_Size(pBoxList);

                // get all class value
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

                    //unsigned char num = (unsigned char)PyInt_AsLong(pBoxListItem);

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
}*/
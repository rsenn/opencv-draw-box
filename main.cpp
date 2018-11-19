#include "image.h"
#include "mjpeg_streaming.h"
#include "json_server.h"
#include <iostream>

using namespace std;

static void * cap;
static image im;
static image **alphabet;

int main()
{
    image im = load_image_color("predictions.jpg", 0, 0);

    im = resize_image(im, 100, 100);

    int w = im.w, h = im.h, c = im.c;
    int im_size = h*w*c;

    printf("w=%d, h=%d, c=%d, im_size=%d\n", w, h, c, im_size);

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
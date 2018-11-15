#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include <stdlib.h>

#include "opencv2/opencv.hpp"
using namespace cv;

#define PERSON_MAX_NUM 10

typedef struct{
    int id;
    int x1, y1, x2, y2;
} person_box;

typedef struct {
    int h;
    int w;
    int c;
    float *data;
} image;

image **load_alphabet();

// ######################### image.c / get_label / draw_label

void fill_cpu(int N, float ALPHA, float *X, int INCX);
void embed_image(image source, image dest, int dx, int dy);
void composite_image(image source, image dest, int dx, int dy);
image tile_images(image a, image b, int dx);
static float get_pixel(image m, int x, int y, int c);
static float get_pixel_extend(image m, int x, int y, int c);
static void set_pixel(image m, int x, int y, int c, float val);
image border_image(image a, int border);
image get_label(image **characters, char *string, int size);
void draw_label(image a, int r, int c, image label, const float *rgb);

// ######################### image.c / draw_box / draw_box_width

image load_image_color(char *filename, int w, int h);
image load_image(char *filename, int w, int h, int c);
void draw_box(image a, int x1, int y1, int x2, int y2, float r, float g, float b);
void draw_box_width(image a, int x1, int y1, int x2, int y2, int w, float r, float g, float b);
image make_empty_image(int w, int h, int c);
image make_image(int w, int h, int c);
void copy_image_into(image src, image dest);
image copy_image(image p);
void constrain_image(image im);
void rgbgr_image(image im);
void free_image(image m);

// ######################### image_opencv.cpp / convert image / show image

IplImage *image_to_ipl(image im);
image ipl_to_image(IplImage* src);
Mat image_to_mat(image im);
image mat_to_image(Mat m);
void *open_video_stream(const char *f, int c, int w, int h, int fps);
image get_image_from_stream(void *p);
image load_image_cv(char *filename, int channels);
int show_image_cv(image im, const char* name, int ms);
void make_window(char *name, int w, int h, int fullscreen);

// #################################

static void add_pixel(image m, int x, int y, int c, float val);
image resize_image(image im, int w, int h);

#endif
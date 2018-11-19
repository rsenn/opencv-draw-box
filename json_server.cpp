#include "json_server.h"

#include <iostream>
#include "opencv2/opencv.hpp"

class JSONServer
{
    SOCKET sock;
    SOCKET maxfd; // record the max value of sock for select() loop
    PORT port;

    fd_set masterfds;
    int timeout; // readfds sock timeout, shutdown after timeout millis.
    int quality; // jpeg compression [1..100]

    // sock: client socket
    // s: constant pointer to a constant char
    // len: length of s
    int _write(int sock, char const * const s, int len)
    {
        if (len < 1)
        {
            len = strlen(s);
        }
        return ::send(sock, s, len, 0);
    }

public:

    // constructor
    // _port: port number
    // _timeout: timeout for select() timeval
    // _quality: jpeg compression [1..100] for cv::imencode (the higher is the better)
    JSONServer(int _port = 0, int _timeout = 200000, int _quality = 30)
        : port(_port)
        , sock(INVALID_SOCKET)
        , timeout(_timeout)
        , quality(_quality)
    {
        signal(SIGPIPE, SIG_IGN); // ignore ISGPIP to avoid client crash and server is forcde to stop
        FD_ZERO(&masterfds); // set readfds all zero

        if(port) open(port); // if port > 0, then create a server with port
    }

    void _read(int sock, char * s, int len)
    {
        recv(sock, s, len, 0);
        printf("%s\n", s);
    }

    // destructor
    ~JSONServer()
    {
        release();
    }

    bool release()
    {
        if (sock != INVALID_SOCKET)
            ::shutdown(sock, 2); // disable receive or send data, like close()
        sock = (INVALID_SOCKET);
        return false;
    }


    bool open(int port)
    {
        sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        SOCKADDR_IN address; // server address information
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        int optval = 1; // set SO_REUSEADDR(1) is true
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

        if (::bind(sock, (SOCKADDR*)&address, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
        {
            std::cerr << "error : couldn't bind sock " << sock << " to port " << port << "!" << std::endl;
            return release();
        }

        if (::listen(sock, 10) == SOCKET_ERROR)
        {
            std::cerr << "error : couldn't listen on sock " << sock << " on port " << port << " !" << std::endl;
            return release();
        }

        FD_ZERO(&masterfds);
        FD_SET(sock, &masterfds);

        maxfd = sock;

        return true;
    }

    bool isOpened()
    {
        return sock != INVALID_SOCKET;
    }

    bool read()
    {
        fd_set readfds = masterfds;

        struct timeval tv = { 0, timeout };

        // nothing broken, there's just noone listening
        if (::select(maxfd + 1, &readfds, NULL, NULL, &tv) <= 0) return true;

        std::vector<uchar> outbuf();
        
        size_t outlen = outbuf.size();

        for (int s = 0; s <= maxfd; s++)
        {
            SOCKLEN_T addrlen = sizeof(SOCKADDR);

            // check whether s is active
            if (!FD_ISSET(s, &readfds))
                continue;
            
            // if s is equal to sock, s is server socket which need to accept and send http header
            if (s == sock)
            {
                SOCKADDR_IN address = {0}; // client address information
                

                SOCKET client = ::accept(sock, (SOCKADDR*)&address, &addrlen);

                if (client == SOCKET_ERROR)
                {
                    std::cerr << "error : couldn't accept connection on sock " << sock << " !" << std::endl;
                    return false;
                }

                char *ip = inet_ntoa(address.sin_addr);

                maxfd = ( maxfd > client ? maxfd : client ); // reset maxfd with the larger one

                FD_SET(client, &masterfds); // insert client fd to masterfds

                std::cerr << "new client " << client << " : " << ip << std::endl;
            }
            else // s is client
            {
                char data[30000];

                memset(data, '\0', sizeof(data));

                _read(s, data, 30000);

                //printf("%s\n", data);

                image im = json_to_image(data);

                show_image_cv(im, "predictions", 0);

                int n = _write(s, (char*)(&outbuf[0]), outlen);

                // std::cerr << " MJPEG-stream sent to ";
                std::cerr << "known client " << s << " " << n << std::endl;

                if (n < 0) // client close or crash
                {
                    std::cerr << "kill client " << s << std::endl;
                    ::shutdown(s, 2);
                    FD_CLR(s, &masterfds);
                }
            }
        }
        return true;
    }
};

image json_to_image(const char * json_str)
{
    int h = 100;
    int w = 100;
    int c = 3;

    int step = 2304;

    image im = make_image(w, h, c);

    unsigned char *data = (unsigned char *)calloc(h*w*c, sizeof(unsigned char));

    // Parsing json
    cJSON *cjson_obj = cJSON_Parse(json_str);

    cJSON *cjson_tmp;
    cJSON *cjson_data;

    cJSON_ArrayForEach(cjson_tmp, cjson_obj)
    {
        //printf("%s\n", cJSON_PrintUnformatted(cjson_tmp));

        /*printf("id = %d\n", (cJSON_GetObjectItem(cjson_tmp, "id"))->valueint);
        printf("x1 = %d\n", (cJSON_GetObjectItem(cjson_tmp, "x1"))->valueint);
        printf("y1 = %d\n", (cJSON_GetObjectItem(cjson_tmp, "y1"))->valueint);
        printf("x2 = %d\n", (cJSON_GetObjectItem(cjson_tmp, "x2"))->valueint);
        printf("y2 = %d\n", (cJSON_GetObjectItem(cjson_tmp, "y2"))->valueint);*/

        cJSON_ArrayForEach(cjson_data, cJSON_GetObjectItem(cjson_tmp, "data"))
        {
            printf("%d ", cjson_data->valueint);
        }
    }



    // Convert to image
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                // ##### to get image information
                //printf("%d ", (int)data[i*step + j*c + k]);
                //printf("%u ", (unsigned char)data[i*step + j*c + k]);

                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }
    return im;
}

/*IplImage *image_to_ipl(image im)
{
    int x,y,c;
    IplImage *disp = cvCreateImage(cvSize(im.w,im.h), IPL_DEPTH_8U, im.c);
    int step = disp->widthStep;
    for(y = 0; y < im.h; ++y){
        for(x = 0; x < im.w; ++x){
            for(c= 0; c < im.c; ++c){
                float val = im.data[c*im.h*im.w + y*im.w + x];
                disp->imageData[y*step + x*im.c + c] = (unsigned char)(val*255);
            }
        }
    }
    return disp;
}

cv::Mat image_to_mat(image im)
{
    image copy = copy_image(im);
    constrain_image(copy);
    if(im.c == 3) rgbgr_image(copy);

    IplImage *ipl = image_to_ipl(copy);
    cv::Mat m = cv::cvarrToMat(ipl, true);
    cvReleaseImage(&ipl);
    free_image(copy);
    return m;
}*/
/*
void send_mjpeg(image im, int port, int timeout, int quality) {
    
    // create only one MJPGWriter object
    static JSONServer mjpeg_writer(port, timeout, quality);

    cv::Mat mat;

    cv::resize(image_to_mat(im), mat, cv::Size(1352, 1013));

    mjpeg_writer.write(mat);
}
*/
#include "socket_server.h"

#include <iostream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

typedef struct frame_with_stamp {
    int frame_stamp;
    std::vector<unsigned char> frame;
} frame_with_stamp;

class ServerSocket
{
    SOCKET server_sock;
    PORT server_port;

    SOCKET maxfd; // record the max value of sock for select() loop

    SELECT_SET masterfds;

    int timeout; // readfds sock timeout, shutdown after timeout millis.
    int quality; // jpeg compression [1..100]

    bool _release()
    {
        if (server_sock != INVALID_SOCKET)
            shutdown(server_sock, 2); // disable receive or send data, like close()
        server_sock = (INVALID_SOCKET);
        return false;
    }

    // @port: server port
    bool _open(int port)
    {
        server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        SOCKADDR_IN address; // server address information
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_family = AF_INET; // tcp
        address.sin_port = htons(port);

        int optval = 1; // set SO_REUSEADDR(1) is true
        setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval); // allow reuse port for some binding error

        // bind server with port
        if (bind(server_sock, (SOCKADDR*)&address, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
        {
            std::cerr << "error : couldn't bind sock " << server_sock << " to port " << port << "!" << std::endl;
            return _release();
        }

        // listen for clinet
        if (listen(server_sock, 10) == SOCKET_ERROR)
        {
            std::cerr << "error : couldn't listen on sock " << server_sock << " on port " << port << " !" << std::endl;
            return _release();
        }

        // initial select fd set
        FD_ZERO(&masterfds);

        // put server fd in select set
        FD_SET(server_sock, &masterfds);

        // set maxfd for loop all socket fd
        maxfd = server_sock;

        return true;
    }

    // @send_sock: client socket
    // @message: string of message
    // @message_len: length of s
    int _write(int send_sock, std::string message, int message_len)
    {
        int send_len = 0;

        //const char *buffer = message.c_str();

        //while(send_len < message_len)
        //{
        //    send_len += send(send_sock, &buffer[send_len], message_len, 0);
        //}

        send_len = send(send_sock, message.c_str(), message_len, 0);

        return send_len;
    }

    int _write_frame(int &send_sock, std::vector<unsigned char> frame, int frame_len)
    {
        int send_len = 0;

        send_len = send(send_sock, frame.data(), frame_len, 0);

        return send_len;
    }

    int _write_len(int send_sock, int messagee_len)
    {
        //std::cout<<"_write_len"<<std::endl;

        int send_len = 0;  

        send_len = send(send_sock, &messagee_len, sizeof(int), 0);

        return send_len;
    }

    // @receive_sock
    // @message_len
    std::string _read(int receive_sock, int message_len)
    {
        std::string message;
        message.clear(); // initial message

        int receive_len = 0;
        int rest_len = message_len;
        char buffer[BUFFER_MAX+1]; // plus one is for big data (4096), because it needs '\0' to decide string end
        memset(buffer, '\0', sizeof(buffer));

        //std::cout<<"should receive: "<<message_len<<std::endl;

        if(message_len <= BUFFER_MAX)
        {
            receive_len = recv(receive_sock, buffer, message_len, 0);
            message += buffer;
        }
        else
        {
            while(rest_len > 0)
            {
                if(rest_len <= BUFFER_MAX)
                {
                    receive_len += recv(receive_sock, buffer, rest_len, 0);
                    rest_len = message_len - receive_len;
                }
                else
                {
                    receive_len += recv(receive_sock, buffer, BUFFER_MAX, 0);
                    rest_len = message_len - receive_len;
                }

                message += buffer;
                memset(buffer, '\0', sizeof(buffer)); // clear buffer for new message

                //std::cout<<rest_len<<std::endl;
            }
        }

        //std::cout<<message<<" ("<<message.size()<<")"<<std::endl;

        return message;
    }

    // @receive_sock
    // @message_len
    // @return: frame vector
    std::vector<unsigned char> _read_frame(int receive_sock, int frame_len)
    {
        std::vector<unsigned char> frame;
        frame.clear(); // initial message

        int receive_len = 0;
        int rest_len = frame_len;
        unsigned char buffer[BUFFER_MAX+1]; // plus one is for big data (4096), because it needs '\0' to decide string end
        memset(buffer, '\0', sizeof(buffer));

        //std::cout<<"should receive: "<<frame_len<<std::endl;

        if(frame_len <= BUFFER_MAX)
        {
            receive_len = recv(receive_sock, buffer, frame_len, 0);
            frame.insert(frame.end(), buffer, buffer + frame_len);
        }
        else
        {
            while(rest_len > 0)
            {
                if(rest_len <= BUFFER_MAX)
                {
                    receive_len = recv(receive_sock, buffer, rest_len, 0);
                    rest_len -= receive_len;
                }
                else
                {
                    receive_len = recv(receive_sock, buffer, BUFFER_MAX, 0);
                    rest_len -= receive_len;
                }

                frame.insert(frame.end(), buffer, buffer + receive_len);
                memset(buffer, '\0', sizeof(buffer)); // clear buffer for new message

                //std::cout<<frame.size()<<std::endl;
                //std::cout<<rest_len<<std::endl;
                //std::cout<<receive_len<<std::endl;
            }
        }

        //std::cout<<message<<" ("<<message.size()<<")"<<std::endl;

        return frame;
    }

    int _read_len(int receive_sock)
    {
        int message_len = 0;
        recv(receive_sock, &message_len, sizeof(int), 0);

        return message_len;
    }

public:

    // constructor
    // _port: port number
    // _timeout: timeout for select() timeval
    // _quality: jpeg compression [1..100] for cvimencode (the higher is the better)
    ServerSocket(int _port = 0, int _timeout = 200000, int _quality = 30)
        : server_port(_port)
        , server_sock(INVALID_SOCKET)
        , timeout(_timeout)
        , quality(_quality)
    {
        signal(SIGPIPE, SIG_IGN); // ignore ISGPIP to avoid client crash and server is forcde to stop
        FD_ZERO(&masterfds); // set readfds all zero

        if(server_port) _open(server_port); // if port > 0, then create a server with port
    }

    // destructor
    ~ServerSocket()
    {
        _release();
    }

    bool isOpened()
    {
        return server_sock != INVALID_SOCKET;
    }

    /*bool start()
    {
        //std::cout<<"waiting..."<<std::endl;

        SELECT_SET readfds = masterfds;

        struct timeval tv = { 0, timeout };

        // nothing broken, there's just noone listening
        if (select(maxfd + 1, &readfds, NULL, NULL, &tv) <= 0) return true;
        
        for (int sock = 0; sock <= maxfd; sock++)
        {
            SOCKLEN_T addrlen = sizeof(SOCKADDR);

            // check whether s is active
            if (!FD_ISSET(sock, &readfds))
                continue;
            
            // if s is equal to server_sock, s is server socket which need to accept client
            if (sock == server_sock)
            {
                SOCKADDR_IN address = {0}; // client address information

                SOCKET client_sock = accept(server_sock, (SOCKADDR*)&address, &addrlen);

                if (client_sock == SOCKET_ERROR)
                {
                    std::cerr << "error : couldn't accept connection on server sock " << server_sock << " !" << std::endl;
                    return false;
                }

                //const char *ip = inet_ntoa(address.sin_addr);
                std::string client_ip = inet_ntoa(address.sin_addr);

                maxfd = ( maxfd > client_sock ? maxfd : client_sock ); // reset maxfd with the larger one

                FD_SET(client_sock, &masterfds); // insert client fd to masterfds

                std::cerr << "new client " << client_sock << " : " << client_ip << std::endl;
            }
            else // s is client
            {

                int message_len = 0;

                message_len = _read_len(sock);

                std::cout<<sock<<" receive: "<<message_len<<std::endl;

                if(message_len > 0)
                {
                    std::vector<unsigned char> frame = _read_frame(sock, message_len);
                    std::cout<<frame.size()<<std::endl;
                }
                else
                {
                    std::cout<<"check"<<std::endl;
                    int n = _write_len(sock, message_len);
                    if (n < 0) // client close or crash
                    {
                        std::cerr << "kill client " << sock << std::endl;
                        shutdown(sock, 2);
                        FD_CLR(sock, &masterfds);
                    }
                }
            }
        }
        return true;
    }*/

    bool start(std::vector<unsigned char> &frame)
    {
        //std::cout<<"waiting..."<<std::endl;

        SELECT_SET readfds = masterfds;

        struct timeval tv = { 0, timeout };

        // nothing broken, there's just noone listening
        if (select(maxfd + 1, &readfds, NULL, NULL, &tv) <= 0) return true;
        
        for (int sock = 0; sock <= maxfd; sock++)
        {
            SOCKLEN_T addrlen = sizeof(SOCKADDR);

            // check whether s is active
            if (!FD_ISSET(sock, &readfds))
                continue;
            
            // if s is equal to server_sock, s is server socket which need to accept client
            if (sock == server_sock)
            {
                SOCKADDR_IN address = {0}; // client address information

                SOCKET client_sock = accept(server_sock, (SOCKADDR*)&address, &addrlen);

                if (client_sock == SOCKET_ERROR)
                {
                    std::cerr << "error : couldn't accept connection on server sock " << server_sock << " !" << std::endl;
                    return false;
                }

                //const char *ip = inet_ntoa(address.sin_addr);
                std::string client_ip = inet_ntoa(address.sin_addr);

                maxfd = ( maxfd > client_sock ? maxfd : client_sock ); // reset maxfd with the larger one

                FD_SET(client_sock, &masterfds); // insert client fd to masterfds

                std::cerr << "new client " << client_sock << " : " << client_ip << std::endl;
            }
            else // s is client
            {

                int frame_len = 0;

                frame_len = _read_len(sock);

                //std::cout<<sock<<" receive: "<<frame_len<<std::endl;

                if(frame_len > 0)
                {
                    frame = _read_frame(sock, frame_len);
                    //std::cout<<frame.size()<<std::endl;
                }
                else
                {
                    //std::cout<<"check"<<std::endl;
                    int n = _write_len(sock, frame_len);
                    if (n < 0) // client close or crash
                    {
                        std::cerr << "kill client " << sock << std::endl;
                        shutdown(sock, 2);
                        FD_CLR(sock, &masterfds);
                        cvDestroyWindow("Demo");
                    }
                }
            }
        }
        return true;
    }

    bool start(std::vector<unsigned char> &frame, int &frame_stamp)
    {
        //std::cout<<"waiting..."<<std::endl;

        SELECT_SET readfds = masterfds;

        struct timeval tv = { 0, timeout };

        // nothing broken, there's just noone listening
        if (select(maxfd + 1, &readfds, NULL, NULL, &tv) <= 0) return true;
        
        for (int sock = 0; sock <= maxfd; sock++)
        {
            SOCKLEN_T addrlen = sizeof(SOCKADDR);

            // check whether s is active
            if (!FD_ISSET(sock, &readfds))
                continue;
            
            // if s is equal to server_sock, s is server socket which need to accept client
            if (sock == server_sock)
            {
                SOCKADDR_IN address = {0}; // client address information

                SOCKET client_sock = accept(server_sock, (SOCKADDR*)&address, &addrlen);

                if (client_sock == SOCKET_ERROR)
                {
                    std::cerr << "error : couldn't accept connection on server sock " << server_sock << " !" << std::endl;
                    return false;
                }

                //const char *ip = inet_ntoa(address.sin_addr);
                std::string client_ip = inet_ntoa(address.sin_addr);

                maxfd = ( maxfd > client_sock ? maxfd : client_sock ); // reset maxfd with the larger one

                FD_SET(client_sock, &masterfds); // insert client fd to masterfds

                std::cerr << "new client " << client_sock << " : " << client_ip << std::endl;
            }
            else // s is client
            {
                frame_stamp = _read_len(sock);

                int frame_len = 0;

                frame_len = _read_len(sock);

                //std::cout<<"frame_stamp : "<<frame_stamp<<std::endl;
                //std::cout<<sock<<" receive: "<<frame_len<<std::endl;

                if(frame_len > 0)
                {
                    frame = _read_frame(sock, frame_len);
                    //std::cout<<frame.size()<<std::endl;
                }
                else
                {
                    //std::cout<<"check"<<std::endl;
                    int n = _write_len(sock, frame_len);
                    if (n < 0) // client close or crash
                    {
                        std::cerr << "kill client " << sock << std::endl;
                        shutdown(sock, 2);
                        FD_CLR(sock, &masterfds);
                        cvDestroyWindow("Demo");
                    }
                }
            }
        }
        return true;
    }
};

/*std::string receive_message(int port, int timeout, int quality)
{
    static ServerSocket server_socket(port, timeout, quality);

    server_socket.start();

    return "";
}*/


int receive_frame(std::vector<unsigned char> &frame, int port, int timeout, int quality)
{
    static ServerSocket server_socket(port, timeout, quality);

    int framp_stamp = 0;

    server_socket.start(frame, framp_stamp);

    return framp_stamp;
}


std::vector<unsigned char> receive_frame(int port, int timeout, int quality)
{
    static ServerSocket server_socket(port, timeout, quality);

    std::vector<unsigned char> frame;

    server_socket.start(frame);

    return frame;
}

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
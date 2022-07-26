#include <ciso646>
#include <string.h>
#ifdef __unix__
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#endif
#include <fcntl.h>
#include <thread>
#include <chrono>
#include "LMS64CProtocol.h"
#include "Logger.h"
#include "threadHelper.h"

namespace lime
{

void LMS64CProtocol::InitRemote()
{
    remoteOpen = true;
    socketFd = -1;
    const int port = 5000;
#ifndef __unix__
    WSADATA wsaData;
    if( int err = WSAStartup(0x0202, &wsaData))
        lime::log(LOG_LEVEL_ERROR, "RemoteControl: WSAStartup error %i\n", err);
#endif

    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFd < 0)
    {
        lime::log(LOG_LEVEL_ERROR, "RemoteControl: socket error %i\n", socketFd);
        return;
    }
#ifdef __unix__
    fcntl(socketFd, F_SETFL, O_NONBLOCK);
#else
    u_long NonBlock = 1;
    ioctlsocket(socketFd, FIONBIO, &NonBlock);
#endif

    struct sockaddr_in host;
    memset((void *)&host, 0, sizeof(sockaddr_in));
    host.sin_family = AF_INET;
    host.sin_addr.s_addr = INADDR_ANY;
    host.sin_port = htons(port);
    if(bind(socketFd, (struct sockaddr*)&host, sizeof(host)) < 0)
    {
        lime::log(LOG_LEVEL_ERROR, "RemoteControl: bind error on port %i\n", port);
        CloseRemote();
        return;
    }

    lime::log(LOG_LEVEL_INFO, "RemoteControl Listening on port: %i\n", port);
    if(int error = listen(socketFd, SOMAXCONN) )
    {
#ifndef __unix__
        error =  WSAGetLastError();
#endif
        lime::log(LOG_LEVEL_ERROR, "RemoteControl listen error %i\n", error);
        CloseRemote();
        return;
    }

    remoteThread = std::thread(&LMS64CProtocol::ProcessConnections, this);
    SetOSThreadPriority(ThreadPriority::LOW, ThreadPolicy::DEFAULT, &remoteThread);
    if(not remoteThread.joinable())
    {
        lime::log(LOG_LEVEL_ERROR, "RemoteControl: Error creating listening thread\n");
        CloseRemote();
        return;
    }
    return;
}

void LMS64CProtocol::CloseRemote()
{
    if (!remoteOpen)
        return;
    remoteOpen = false;

    if(remoteThread.joinable())
        remoteThread.join();

    if(socketFd > 0)
    {
#ifndef __unix__
    //shutdown(socketFd, SD_BOTH);
    closesocket(socketFd);
#else
    shutdown(socketFd, SHUT_RDWR);
    close(socketFd);
#endif
    }

#ifndef __unix__
    WSACleanup();
#endif
}


void LMS64CProtocol::ProcessConnections()
{
    struct sockaddr_in cli_addr;
#ifndef __unix__
    SOCKET clientFd;
    int clilen = sizeof(cli_addr);
#else
    int clientFd;
    unsigned int clilen = sizeof(cli_addr);
#endif // __unix__

    while(remoteOpen && socketFd >= 0)
    {
        memset(&cli_addr, 0, sizeof(cli_addr));
        clientFd = accept(socketFd, (struct sockaddr*)&cli_addr, &clilen);

        //if(clientFd == EAGAIN || clientFd == EWOULDBLOCK)
            //continue;
        if(clientFd < 0)
        {
            continue;
        }

        bool connected = true;

        const int msgSize = 64;
        char data[msgSize] = {0};

        while(connected && remoteOpen)
        {
            unsigned int r = 0;
            do
            {
                int brecv = recv(clientFd, data+r, msgSize-r, 0);
                r += brecv;
                if(brecv == 0)
                {
                    connected = false;
                    break;
                }
                if(brecv < 0 )
                {
#ifndef __unix__
                    brecv = WSAGetLastError();
#endif
                    lime::log(LOG_LEVEL_ERROR, "RemoteControl recv with error: %d\n", brecv);
                    connected = false;
                    break;
                }
            }
            while(r < msgSize);

            if(not connected)
                break;

            mControlPortLock.lock();
            Write((unsigned char*)data, msgSize);
            Read((unsigned char*)data, msgSize);
            mControlPortLock.unlock();

            int bsent = 0;
            while(bsent < msgSize)
            {
                int r = send(clientFd, data+bsent, msgSize-bsent, 0);
                if(r < 0)
                {
                    lime::log(LOG_LEVEL_ERROR, "RemoteControl send with error: %d\n", r);
                    connected = false;
                    break;
                }
                bsent += r;
            }
        }
        if (clientFd >= 0)
        {
#ifndef __unix__
            //shutdown(socketFd, SD_BOTH);
            closesocket(clientFd);
#else
            shutdown(clientFd, SHUT_RDWR);
            close(clientFd);
#endif
        }
    }
    return;
}


}

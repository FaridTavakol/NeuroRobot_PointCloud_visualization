// server is supposed to be the NeuroKinematics code

#include <iostream>
#include <math.h>
#include <cstdlib>

#include "igtlOSUtil.h"
#include "igtlStringMessage.h"
#include "igtlServerSocket.h"
#include "igtlPointMessage.h"

#define N_STRINGS 1

const char *testString[N_STRINGS] = {
    "Waiting for your request!\n"};
const char *client_replys[N_STRINGS] = {
    "I heard you!\n"};

int main(int argc, char *argv[])
{
    //------------------------------------------------------------
    // Parse Arguments

    if (argc != 3) // check number of arguments
    {
        // If not correct, print usage
        std::cerr << "Usage: " << argv[0] << " <port> <fps>" << std::endl;
        std::cerr << "    <port>     : Port # (18944 in Slicer default)" << std::endl;
        std::cerr << "    <fps>      : Frequency (fps) to send string" << std::endl;
        exit(0);
    }

    int port = atoi(argv[1]);
    double fps = atof(argv[2]);
    int interval = (int)(1000.0 / fps);

    igtl::StringMessage::Pointer stringMsg;
    stringMsg = igtl::StringMessage::New();
    stringMsg->SetDeviceName("StringMessage");

    igtl::ServerSocket::Pointer serverSocket;
    serverSocket = igtl::ServerSocket::New();
    int r = serverSocket->CreateServer(port);

    if (r < 0)
    {
        std::cerr << "Cannot create a server socket." << std::endl;
        exit(0);
    }

    igtl::Socket::Pointer socket;
    // Waiting for Connection
    // socket = serverSocket->WaitForConnection(3000);
    // std::cout << "Waiting for connection\n";
    int flag{0};
    igtl::MessageHeader::Pointer hdrMsg = igtl::MessageHeader::New(); // This message is for the server incoming headers

    while (1)
    {
        //------------------------------------------------------------
        // Waiting for Connection
        socket = serverSocket->WaitForConnection(1000);
        std::cout << "Waiting for connection\n";

        // First stage (sending string to client)
        if (socket.IsNotNull() && flag == 0) // if client connected
        {
            //------------------------------------------------------------
            // loop
            std::cout << "Sending string: " << testString[N_STRINGS] << std::endl;
            stringMsg->SetDeviceName("StringMessage");
            stringMsg->SetString(testString[N_STRINGS]);
            stringMsg->Pack();
            socket->Send(stringMsg->GetPackPointer(), stringMsg->GetPackSize());
            igtl::Sleep(1000); // wait

            // second stage (waiting to get a reply from the client)
            while (1)
            {
                hdrMsg->InitPack();
                int r = socket->Receive(hdrMsg->GetPackPointer(), hdrMsg->GetPackSize());
                // check message
                if (r == 0 || r != hdrMsg->GetPackSize())
                {
                    // socket->CloseSocket();
                    continue;
                }
                hdrMsg->Unpack();
                igtl::StringMessage::Pointer strMsg(igtl::StringMessage::New());
                strMsg->SetMessageHeader(hdrMsg);
                strMsg->AllocatePack();
                socket->Receive(strMsg->GetPackBodyPointer(), strMsg->GetPackBodySize());
                int c = strMsg->Unpack();
                std::cout << "Message from the client: " << strMsg->GetString() << std::endl;
                if (strcmp(strMsg->GetString(), "send_point") == 0)
                {
                    std::cout << "I received your request!" << std::endl;
                    igtl::PointMessage::Pointer pointMsg = igtl::PointMessage::New();
                    pointMsg->SetDeviceName("PointSender");
                    // Create a point and send to the client
                    igtl::PointElement::Pointer point0;
                    point0 = igtl::PointElement::New();
                    point0->SetName("POINT_0");
                    point0->SetGroupName("GROUP_0");
                    point0->SetRGBA(120, 125, 255, 1);
                    igtlFloat32 x{0.}, y{0.}, z{0.};
                    std::cout << "Enter x, y, z :\n";
                    std::cin >> x >> y >> z;
                    point0->SetPosition(x, y, z);
                    point0->SetRadius(15.0);
                    point0->SetOwner("IMAGE_0");
                    pointMsg->AddPointElement(point0);
                    pointMsg->Pack();
                    socket->Send(pointMsg->GetPackPointer(), pointMsg->GetPackSize());
                    break;
                }
            }
            flag++;
        }
        std::cout << "socket status is :" << socket.IsNotNull() << std::endl;
        std::cout << "Flag is :" << flag << std::endl;
        if (socket.IsNotNull() && flag == 1) // if client connected
        {
            std::cout << "I am here now!\n";
            flag++;
        }
        if (!socket.IsNotNull() && flag >= 1) // if client connected
        {
            while (1)
            {
                std::cout << "I am in the third loop now!\n";
                igtl::Sleep(1000);
            }
            flag++;
        }
    }
    //------------------------------------------------------------
    // Close connection (The example code never reachs to this section ...)
    std::cout << "\nSocket closed!\n";
    socket->CloseSocket();
}

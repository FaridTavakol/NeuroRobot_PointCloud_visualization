#include <iostream>
#include <math.h>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <cstring>

#include "igtlOSUtil.h"
#include "igtlPointMessage.h"
#include "igtlServerSocket.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: "
                  << " <port> <fps>" << std::endl;
        std::cerr << " <port> : Port # (18944 in Slicer default)" << std::endl;
        exit(0);
    }

    int port = atoi(argv[1]);

    igtl::ServerSocket::Pointer serversocket;
    serversocket = igtl::ServerSocket::New();
    int r = serversocket->CreateServer(port);

    if (r < 0)
    {
        std::cerr << "Cannot establish a server socket.\n ";
        exit(0);
    }
    else
    {
        std::cerr << "Connection established!\n";
    }

    //we need to create the message type listener
    igtl::Socket::Pointer socket;

    while (1)
    {
        //waiting for a connection
        socket = serversocket->WaitForConnection(1000);
        if (socket.IsNotNull())
        {
            // connection is established
            // Allocating transform message class
            igtl::PointMessage::Pointer pointMsg;
            pointMsg = igtl::PointMessage::New();
            pointMsg->SetDeviceName("PointSender");
            //_____________________________________
            //create 1st point
            igtl::PointElement::Pointer point0;
            point0 = igtl::PointElement::New();
            point0->SetName("POINT_0");
            point0->SetGroupName("GROUP_0");
            point0->SetRGBA(255, 65, 170, 1);
            point0->SetPosition(10.0, 20.0, 30.0);
            point0->SetRadius(45.0);
            point0->SetOwner("IMAGE_0");
            //---------------------------
            // Create 2nd point
            igtl::PointElement::Pointer point1;
            point1 = igtl::PointElement::New();
            point1->SetName("POINT_1");
            point1->SetGroupName("GROUP_0");
            point1->SetRGBA(25, 25, 255, 1);
            point1->SetPosition(40.0, 50.0, 60.0);
            point1->SetRadius(45.0);
            point1->SetOwner("IMAGE_0");
            //---------------------------
            // Create 3rd point
            igtl::PointElement::Pointer point2;
            point2 = igtl::PointElement::New();
            point2->SetName("POINT_2");
            point2->SetGroupName("GROUP_0");
            point2->SetRGBA(48, 241, 57, 1);
            point2->SetPosition(70.0, 80.0, 90.0);
            point2->SetRadius(45.0);
            point2->SetOwner("IMAGE_0");
            // pack into a Point message
            pointMsg->AddPointElement(point0);
            pointMsg->AddPointElement(point1);
            pointMsg->AddPointElement(point2);
            pointMsg->Pack();
            // ------------------------------------------------
            // send
            socket->Send(pointMsg->GetPackPointer(), pointMsg->GetPackSize());
            //----------------------------closing the socket
            socket->CloseSocket();
            std::cerr << "Closing the socket\n";
        }
    }
}
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <string>

#include "igtlOSUtil.h"
#include "igtlPointMessage.h"
#include "igtlClientSocket.h"

int main(int argc, char *argv[])
{
    //----------------------------------------------------
    // Parse arguments
    if (argc != 3)
    {
        // if not correct, print usage
        std::cerr << "Usage: " << argv[0] << " <hostname> <port> <fps>" << std::endl;
        std::cerr << " <hostname>   : IP or host name" << std::endl;
        std::cerr << " <port>       : Port # (18944) in Slicer default" << std::endl;
        exit(EXIT_FAILURE);
    }
    char *hostname = argv[1];
    int port = atoi(argv[2]);
    // _______________________________________________________
    //Establishing connection

    igtl::ClientSocket::Pointer socket;
    socket = igtl::ClientSocket::New();

    /// Connects to host. Returns 0 on success, -1 on error.
    int r = socket->ConnectToServer(hostname, port);

    if (r != 0)
    {
        std::cout << " Connection can not be established! " << std::endl;
        exit(0);
    }
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
    // while (1)
    // {
    //     socket->Send(pointMsg->GetPackPointer(), pointMsg->GetPackSize());
    // }
    socket->Send(pointMsg->GetPackPointer(), pointMsg->GetPackSize());
    std::cout << pointMsg->GetPackSize() << std::endl;

    //----------------------------closing the socket
    socket->CloseSocket();
}

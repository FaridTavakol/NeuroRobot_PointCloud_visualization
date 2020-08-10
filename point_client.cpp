#include <iostream>
#include <math.h>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <cstring>

#include "igtlOSUtil.h"
#include "igtlPointMessage.h"
#include "igtlClientSocket.h"

int ReceivePoint(igtl::Socket *socket, igtl::MessageHeader *header);
int ReceivePosition(igtl::Socket *socket, igtl::MessageHeader *header);

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
        std::cerr << " Connection can not be established! " << std::endl;
        exit(0);
    }
    else
    {
        std::cerr << "Connection established!\n";
    }
    //------------------------------------------------------------
    // Create a message buffer to receive header
    igtl::MessageHeader::Pointer headerMsg;
    headerMsg = igtl::MessageHeader::New();
    //-----------------------------------------
    //Allocate a timestamp
    igtl::TimeStamp::Pointer ts;
    ts = igtl::TimeStamp::New();

    // while (1)
    // {
    //loop
    for (int i = 0; i < 1; i++)
    {
        //initialize receive buffer
        headerMsg->InitPack();
        //receive generic header from the socket
        int r = socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
        if (r == 0)
        {
            std::cerr << "Error has occured " << std::endl;
            socket->CloseSocket();
        }
        if (r != headerMsg->GetPackSize())
        {
            continue;
        }

        // deserialize the header
        headerMsg->Unpack();

        // Get time stamp
        igtlUint32 sec;
        igtlUint32 nanosec;
        headerMsg->GetTimeStamp(ts);
        ts->GetTimeStamp(&sec, &nanosec);
        std::cerr << "Time stamp: "
                  << sec << "." << std::setw(9) << std::setfill('0')
                  << nanosec << std::endl;
        // check data type of the received message
        if (strcmp(headerMsg->GetDeviceType(), "POINT") == 0)
        {
            ReceivePoint(socket, headerMsg);
        }
        else
        {
            //if data type is unknown skip reading.
            std::cerr << "receiving : " << headerMsg->GetDeviceType() << std::endl;
            std::cerr << "Size : " << headerMsg->GetBodySizeToRead() << std::endl;
            socket->Skip(headerMsg->GetBodySizeToRead(), 0);
        }
    }
    // }
    socket->CloseSocket();
}

int ReceivePoint(igtl::Socket *socket, igtl::MessageHeader *header)
{

    std::cerr << "Receiving POINT data type." << std::endl;

    // Create a message buffer to receive transform data
    igtl::PointMessage::Pointer pointMsg;
    pointMsg = igtl::PointMessage::New();
    pointMsg->SetMessageHeader(header);
    pointMsg->AllocatePack();

    // Receive transform data from the socket
    socket->Receive(pointMsg->GetPackBodyPointer(), pointMsg->GetPackBodySize());

    // Deserialize the transform data
    // If you want to skip CRC check, call Unpack() without argument.
    int c = pointMsg->Unpack(1);

    if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
    {
        int nElements = pointMsg->GetNumberOfPointElement();
        for (int i = 0; i < nElements; i++)
        {
            igtl::PointElement::Pointer pointElement;
            pointMsg->GetPointElement(i, pointElement);

            igtlUint8 rgba[4];
            pointElement->GetRGBA(rgba);

            igtlFloat32 pos[3];
            pointElement->GetPosition(pos);

            std::cerr << "========== Element #" << i << " ==========" << std::endl;
            std::cerr << " Name      : " << pointElement->GetName() << std::endl;
            std::cerr << " GroupName : " << pointElement->GetGroupName() << std::endl;
            std::cerr << " RGBA      : ( " << (int)rgba[0] << ", " << (int)rgba[1] << ", " << (int)rgba[2] << ", " << (int)rgba[3] << " )" << std::endl;
            std::cerr << " Position  : ( " << std::fixed << pos[0] << ", " << pos[1] << ", " << pos[2] << " )" << std::endl;
            std::cerr << " Radius    : " << std::fixed << pointElement->GetRadius() << std::endl;
            std::cerr << " Owner     : " << pointElement->GetOwner() << std::endl;
            std::cerr << "================================" << std::endl;
        }
    }

    return 1;
}

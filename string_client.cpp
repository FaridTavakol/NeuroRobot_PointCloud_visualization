/*=========================================================================

  Program:   OpenIGTLink -- Example for Data Receiving Client Program
  Language:  C++

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
//client is supposed to be the 3D slicer
#include "igtlPointMessage.h"
#include <iostream>
#include <iomanip>
#include <math.h>
#include <cstdlib>
#include <cstring>

#include "igtlOSUtil.h"
#include "igtlMessageHeader.h"
#include "igtlClientSocket.h"
#include "igtlStringMessage.h"

#define N_elements 1
const char *request[N_elements] = {"send_point"};

int ReceiveString(igtl::Socket *socket, igtl::MessageHeader::Pointer &header);
int ReceivePoint(igtl::Socket *socket, igtl::MessageHeader::Pointer &header);

int main(int argc, char *argv[])
{
    //------------------------------------------------------------
    // Parse Arguments

    if (argc != 3) // check number of arguments
    {
        // If not correct, print usage
        std::cerr << "    <hostname> : IP or host name" << std::endl;
        std::cerr << "    <port>     : Port # (18944 in Slicer default)" << std::endl;
        exit(0);
    }

    char *hostname = argv[1];
    int port = atoi(argv[2]);

    //------------------------------------------------------------
    // Establish Connection

    igtl::ClientSocket::Pointer socket;
    socket = igtl::ClientSocket::New();
    int r = socket->ConnectToServer(hostname, port);

    if (r != 0)
    {
        std::cerr << "Cannot connect to the server." << std::endl;
        exit(0);
    }

    //------------------------------------------------------------
    // Create a message buffer to receive header
    igtl::MessageHeader::Pointer headerMsg;
    headerMsg = igtl::MessageHeader::New();

    //------------------------------------------------------------
    // Allocate a time stamp
    igtl::TimeStamp::Pointer ts;
    ts = igtl::TimeStamp::New();
    int counter = 0;
    int b = 0;
    igtl::StringMessage::Pointer stringMsg;
    stringMsg = igtl::StringMessage::New();
    while (1)
    {
        if (counter == 0)
        {
            // Initialize receive buffer
            headerMsg->InitPack();
            // Receive generic header from the socket
            int r = socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
            if (r == 0) //if no data is being sent from the server
            {
                socket->CloseSocket();
                exit(0);
            }
            if (r != headerMsg->GetPackSize())
            {
                continue;
            }

            // Deserialize the header
            headerMsg->Unpack();

            // Get time stamp
            igtlUint32 sec;
            igtlUint32 nanosec;

            headerMsg->GetTimeStamp(ts);
            ts->GetTimeStamp(&sec, &nanosec);

            std::cerr << "Name: " << headerMsg->GetDeviceName() << std::endl;
            std::cerr << "Time stamp: "
                      << sec << "." << std::setw(9) << std::setfill('0')
                      << nanosec << std::endl;

            // Check data type and receive data body

            if (strcmp(headerMsg->GetDeviceType(), "STRING") == 0)
            {
                ReceiveString(socket, headerMsg);
            }
            else
            {
                std::cerr << "Receiving : " << headerMsg->GetDeviceType() << std::endl;
                socket->Skip(headerMsg->GetBodySizeToRead(), 0);
            }
            std::cout << "\n receiving the next string\n";

            counter++;
        }
        if (counter == 1) // second stage of (sending request to the server)
        {

            stringMsg->SetDeviceName("StringMessage");
            std::cout << "Sending string: " << request[0] << std::endl;
            stringMsg->SetString(request[0]);
            stringMsg->Pack();
            socket->Send(stringMsg->GetPackPointer(), stringMsg->GetPackSize());
            igtl::Sleep(1000); // wait

            //waiting for reply from the server
            while (1)
            {
                std::cout << "waiting for a reply from the server!" << std::endl;
                headerMsg->InitPack();
                int r = socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());

                if (r == 0 || r < 0)
                {
                    continue;
                }
                headerMsg->Unpack();
                // Get time stamp
                igtlUint32 sec;
                igtlUint32 nanosec;

                headerMsg->GetTimeStamp(ts);
                ts->GetTimeStamp(&sec, &nanosec);

                std::cerr << "Name: " << headerMsg->GetDeviceName() << std::endl;
                std::cerr << "Time stamp: "
                          << sec << "." << std::setw(9) << std::setfill('0')
                          << nanosec << std::endl;

                // Check data type and receive data body

                if (strcmp(headerMsg->GetDeviceType(), "STRING") == 0)
                {
                    ReceiveString(socket, headerMsg);
                    break;
                }
                else if (strcmp(headerMsg->GetDeviceType(), "POINT") == 0)
                {
                    ReceivePoint(socket, headerMsg);
                    break;
                }
            }

            counter++;
        }
        if (counter > 1)
        {
            while (1)
            {
                std::cerr << "In the while loop!" << std::endl;
                igtl::Sleep(1000); // wait
            }
        }
    }

    //------------------------------------------------------------
    // Close connection (The example code never reaches this section ...)

    socket->CloseSocket();
}
//======================================================================================
int ReceiveString(igtl::Socket *socket, igtl::MessageHeader::Pointer &header)
{

    std::cerr << "Receiving STRING data type." << std::endl;

    // Create a message buffer to receive transform data
    igtl::StringMessage::Pointer stringMsg;
    stringMsg = igtl::StringMessage::New();
    stringMsg->SetMessageHeader(header);
    stringMsg->AllocatePack();

    // Receive transform data from the socket
    socket->Receive(stringMsg->GetPackBodyPointer(), stringMsg->GetPackBodySize());

    // Deserialize the transform data
    // If you want to skip CRC check, call Unpack() without argument.
    int c = stringMsg->Unpack(1);

    if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
    {
        std::cerr << "Encoding: " << stringMsg->GetEncoding() << "; "
                  << "String: " << stringMsg->GetString() << std::endl
                  << std::endl;
    }

    return 1;
}
int ReceivePoint(igtl::Socket *socket, igtl::MessageHeader::Pointer &header)
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
            std::cerr << "================================" << std::endl
                      << std::endl;
        }
    }

    return 1;
}

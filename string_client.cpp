/*=========================================================================

  Program:   OpenIGTLink -- Example for Data Receiving Client Program
  Language:  C++

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <iostream>
#include <iomanip>
#include <math.h>
#include <cstdlib>
#include <cstring>

#include "igtlOSUtil.h"
#include "igtlMessageHeader.h"
#include "igtlClientSocket.h"
#include "igtlStringMessage.h"
#define N_STRINGS 5

const char *testString[N_STRINGS] = {
    "OpenIGTLink",
    "Network",
    "Communication",
    "Protocol",
    "Image Guided Therapy",
};
#define N 1
const char *prompts[N] = {
    "Enter your message to the client",
};
int ReceiveString(igtl::Socket *socket, igtl::MessageHeader::Pointer &header);

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
        if (counter < 5)
        {
            // Initialize receive buffer
            headerMsg->InitPack();
            // Receive generic header from the socket
            int r = socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
            std::cout << "Im here";

            if (r == 0) //if no data is being sent from the server
            {
                socket->CloseSocket();
                exit(0);
            }
            if (r != headerMsg->GetPackSize())
            {
                b++;
                std::cout << "\n still r is not equal to pack size!\n"
                          << "b is :" << b;
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
        }
        else if (counter >= 5 && counter < 6) // now we are sending data to the server
        {
            for (int i = 0; i < 5; i++)
            {
                stringMsg->SetDeviceName("StringMessage");
                std::cout << "Sending string: " << testString[i] << std::endl;
                stringMsg->SetString(testString[i]);
                stringMsg->Pack();
                socket->Send(stringMsg->GetPackPointer(), stringMsg->GetPackSize());
                igtl::Sleep(1000); // wait
                i = (i + 1) % N_STRINGS;
            }
            //
            // after sending all 5 strings to the server
        }
        if (counter == 6)
        {
            stringMsg->SetDeviceName("StringMessage");
            std::cout << "Sending string: " << prompts[0] << std::endl;
            stringMsg->SetString(prompts[0]);
            stringMsg->Pack();
            socket->Send(stringMsg->GetPackPointer(), stringMsg->GetPackSize());
            igtl::Sleep(1000);
        }
        counter++;
    }

    //------------------------------------------------------------
    // Close connection (The example code never reaches this section ...)

    socket->CloseSocket();
}

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

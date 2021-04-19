#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pty.h>
#include <vector>
#include <csignal>
#include <fstream>

enum RuntimeErrors
{
    OPENPTY_CANT_CREATE_INTERFACE = 1,
    TOO_FEW_ARGS = 2,
    CANT_CREATE_TTYCLONER_TMP_FILE = 3,
};

using namespace std;

bool stopApplication = false;

void SignalHandler(int signal)
{
    stopApplication = true;
}

int main(int argc, char* argv[])
{
    std::signal(SIGABRT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);
    std::signal(SIGKILL, SignalHandler);
    std::signal(SIGSEGV, SignalHandler);

    int numOfTtyIntrefaces;

    if (argc != 2)
    {
        printf("You have to set up first startup argument to number of tty interface clones, e.g.:\n\
./TtyCloner 2\n\
OR\n\
./TtyCloner 5\n\
for 2 OR 5 interface clones.");
        exit(TOO_FEW_ARGS);
    }
    else
    {
        numOfTtyIntrefaces = atol(argv[1]);
    }

    char buffer = 0;

    int ttyDevice = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    struct termios options;
    tcgetattr(ttyDevice, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(ttyDevice, TCIFLUSH);
    tcsetattr(ttyDevice, TCSANOW, &options);

    int status = 0;
    vector<int> masterDev, slaveDev;

    ofstream clonedInterfacesFile("/tmp/ttyCloner.txt");
    if (!clonedInterfacesFile.good())
    {
        close(ttyDevice);
        exit(CANT_CREATE_TTYCLONER_TMP_FILE);
    }

    for (int i = 0; !stopApplication && i < numOfTtyIntrefaces; i++)
    {
        int master, slave;
        status = openpty(&master, &slave, NULL, NULL, NULL);
        if (status < 0)
        {
            for (size_t i = 0; i < masterDev.size(); i++)
            {
                close(masterDev[i]);
                close(slaveDev[i]);
            }

            clonedInterfacesFile.close();
            std::remove("/tmp/ttyCloner.txt");

            perror("Openpty can't create interface: ");
            exit(OPENPTY_CANT_CREATE_INTERFACE);
        }

        fcntl(master, F_SETFL, FNDELAY); // Setting non blocking mode to masterDev
        fcntl(slave, F_SETFL, FNDELAY); // Setting non blocking mode to masterDev

        masterDev.push_back(master);
        slaveDev.push_back(slave);

        //printf("Slave%d name: %s\n", i, ttyname(slaveDev[slaveDev.size() - 1]));
        clonedInterfacesFile << ttyname(slaveDev[slaveDev.size() - 1]) << std::endl;
    }

    clonedInterfacesFile.close();

    while (!stopApplication)
    {
        ssize_t receivedBytes = read(ttyDevice, &buffer, 1);

        /// <summary>
        /// Read from ttyDevice and write to all masters
        /// </summary>
        /// <returns></returns>
        if (receivedBytes > 0)
        {
            /*putchar(buffer);
            fflush(stdout);*/

            for (size_t i = 0; i < masterDev.size(); i++)
            {
                write(masterDev[i], &buffer, 1);
            }
        }
        ///

        /// <summary>
        /// Read from all masters and write to ttyDevice
        /// </summary>
        /// <returns></returns>
        for (size_t i = 0; i < masterDev.size(); i++)
        {
            char slaveBuffer[4095];
            size_t j = 0;
            receivedBytes = read(masterDev[i], &buffer, 1);
            while (receivedBytes > 0 && j < 4095)
            {
                slaveBuffer[j] = buffer;
                ++j;

                receivedBytes = read(masterDev[i], &buffer, 1);
            }

            if (j > 0)
            {
                /*for (size_t k = 0; k < j; k++)
                {
                    putchar(slaveBuffer[k]);
                }
                fflush(stdout);*/

                write(ttyDevice, &slaveBuffer, j);
            }
        }
        ///
    }

    for (size_t i = 0; i < masterDev.size(); i++)
    {
        close(masterDev[i]);
        close(slaveDev[i]);
    }

    close(ttyDevice);

    getchar();
    return 0;
}
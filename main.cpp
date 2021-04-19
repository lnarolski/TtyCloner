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
	char* clonedTtyInterface;
	unsigned int baudrate = B9600;

	if (argc < 3)
	{
		printf("You have to set up first startup argument to number of tty interface clones and the second one with cloned Tty intreface and the third one with baudrate (default 9600 Bd), e.g.:\n\
./TtyCloner 2 /dev/ttyS0 19200\n\
OR\n\
./TtyCloner 5 /dev/ttyUSB0\n\
for 2 '/dev/ttyS0' with 19200 Bd baudrate OR 5 '/dev/ttyUSB0' with 9600 Bd baudrate interface clones.");
		exit(TOO_FEW_ARGS);
	}
	else
	{
		numOfTtyIntrefaces = atol(argv[1]);
		clonedTtyInterface = argv[2];
	}

	if (argc == 4)
	{
		unsigned int argBaudrate = atol(argv[3]);
		switch (argBaudrate)
		{
		case 0:
			baudrate = B0;
			break;
		case 50:
			baudrate = B50;
			break;
		case 75:
			baudrate = B75;
			break;
		case 110:
			baudrate = B110;
			break;
		case 134:
			baudrate = B134;
			break;
		case 150:
			baudrate = B150;
			break;
		case 200:
			baudrate = B200;
			break;
		case 300:
			baudrate = B300;
			break;
		case 600:
			baudrate = B600;
			break;
		case 1200:
			baudrate = B1200;
			break;
		case 1800:
			baudrate = B1800;
			break;
		case 2400:
			baudrate = B2400;
			break;
		case 4800:
			baudrate = B4800;
			break;
		case 9600:
			baudrate = B9600;
			break;
		case 19200:
			baudrate = B19200;
			break;
		case 38400:
			baudrate = B38400;
			break;
		case 57600:
			baudrate = B57600;
			break;
		case 115200:
			baudrate = B115200;
			break;
		case 230400:
			baudrate = B230400;
			break;
		case 460800:
			baudrate = B460800;
			break;
		case 500000:
			baudrate = B500000;
			break;
		case 576000:
			baudrate = B576000;
			break;
		case 921600:
			baudrate = B921600;
			break;
		case 1000000:
			baudrate = B1000000;
			break;
		case 1152000:
			baudrate = B1152000;
			break;
		case 1500000:
			baudrate = B1500000;
			break;
		case 2000000:
			baudrate = B2000000;
			break;
		case 2500000:
			baudrate = B2500000;
			break;
		case 3000000:
			baudrate = B3000000;
			break;
		case 3500000:
			baudrate = B3500000;
			break;
		case 4000000:
			baudrate = B4000000;
			break;
		default:
			break;
		}
	}

	char buffer = 0;

	int ttyDevice = open(clonedTtyInterface, O_RDWR | O_NOCTTY | O_NDELAY);
	struct termios options;
	tcgetattr(ttyDevice, &options);
	options.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(ttyDevice, TCIFLUSH);
	tcsetattr(ttyDevice, TCSANOW, &options);

	int status = 0;
	std::vector<int> masterDev, slaveDev;

	std::ofstream clonedInterfacesFile("/tmp/ttyCloner.txt");
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

	std::remove("/tmp/ttyCloner.txt");

	return 0;
}
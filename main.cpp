#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pty.h>
#include <vector>
#include <csignal>
#include <fstream>
#include "./cxxopts.hpp"
#include <exception>
#include <iostream>

enum RuntimeErrors
{
	OPENPTY_CANT_CREATE_INTERFACE = 1,
	CANT_PARSE_ARGS = 2,
	CANT_CREATE_TTYCLONER_TMP_FILE = 3,
	UNSUPPORTED_BAUDRATE = 4,
	REMOVE_TMP_FILE_ERROR = 5,
	OPENTTY_CANT_OPEN_INTERFACE = 6,
};

bool stopApplication = false;

void SignalHandler(int signal)
{
	stopApplication = true;
}

int main(int argc, char* argv[])
{
	cxxopts::Options consoleOptions("TtyCloner", "Clone tty device to few pseudo ttys without worrying about blocking cloned device");
	consoleOptions.allow_unrecognised_options();
	consoleOptions.add_options()
		("b,baudrate", "Set baudrate", cxxopts::value<int>() -> default_value("9600"))
		("n,number", "Number of new pty interfaces", cxxopts::value<int>())
		("i,interface", "Tty interface to clone", cxxopts::value<std::string>())
		("f,file", "Create file /tmp/TtyCloner.txt with list of created pty interfaces")
		("h,help", "Show this help")
	;

	cxxopts::ParseResult result;
	try
	{
		result = consoleOptions.parse(argc, argv);
	}
	catch (const std::exception&)
	{
		return CANT_PARSE_ARGS;
	}

	if (result.count("help") || !result.count("number") || !result.count("interface"))
	{
		std::cout << consoleOptions.help() << std::endl;
		exit(0);
	}

	std::signal(SIGABRT, SignalHandler);
	std::signal(SIGTERM, SignalHandler);
	std::signal(SIGKILL, SignalHandler);
	std::signal(SIGSEGV, SignalHandler);

	int numOfTtyIntrefaces;
	const char* clonedTtyInterface;
	unsigned int baudrate;
	unsigned int argBaudrate;
	useconds_t sleepTime_ms = 200;

	numOfTtyIntrefaces = result["number"].as<int>();
	clonedTtyInterface = result["interface"].as<std::string>().c_str();
	argBaudrate = result["baudrate"].as<int>();

	sleepTime_ms = (useconds_t)((((double)19200.0) / argBaudrate) * 100);
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
		return UNSUPPORTED_BAUDRATE;
		break;
	}

	char buffer[4095];

	int ttyDevice = open(clonedTtyInterface, O_RDWR | O_NOCTTY | O_NDELAY);

	if (ttyDevice == -1)
	{
		return OPENTTY_CANT_OPEN_INTERFACE;
	}

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

	std::ofstream clonedInterfacesFile;
	if (result.count("file"))
	{
		clonedInterfacesFile.open("/tmp/TtyCloner.txt");

		if (!clonedInterfacesFile.good())
		{
			close(ttyDevice);
			exit(CANT_CREATE_TTYCLONER_TMP_FILE);
		}
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

			if (result.count("file"))
			{
				clonedInterfacesFile.close();
				std::remove("/tmp/TtyCloner.txt");
			}

			perror("Openpty can't create interface: ");
			exit(OPENPTY_CANT_CREATE_INTERFACE);
		}

		fcntl(master, F_SETFL, FNDELAY); // Setting non blocking mode to masterDev
		fcntl(slave, F_SETFL, FNDELAY); // Setting non blocking mode to masterDev

		masterDev.push_back(master);
		slaveDev.push_back(slave);

		//printf("Slave%d name: %s\n", i, ttyname(slaveDev[slaveDev.size() - 1]));

		if (result.count("file"))
			clonedInterfacesFile << ttyname(slaveDev[slaveDev.size() - 1]) << std::endl;
	}
	if (result.count("file"))
		clonedInterfacesFile.close();

	while (!stopApplication)
	{
		usleep(sleepTime_ms * 1000);
		ssize_t receivedBytes = read(ttyDevice, &buffer, 4095);

		/// Read from ttyDevice and write to all masters
		if (receivedBytes > 0)
		{
			for (size_t i = 0; i < masterDev.size(); i++)
			{
				write(masterDev[i], &buffer, receivedBytes);
			}
		}
		///

		/// Read from all masters and write to ttyDevice
		for (size_t i = 0; i < masterDev.size(); i++)
		{
			receivedBytes = read(masterDev[i], &buffer, 4095);
			if (receivedBytes > 0)
			{
				write(ttyDevice, &buffer, receivedBytes);
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

	if (result.count("file"))
	{
		if (std::remove("/tmp/TtyCloner.txt"))
		{
			return REMOVE_TMP_FILE_ERROR;
		}
	}

	return 0;
}
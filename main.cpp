#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

bool stopApplication = false;

int main()
{
    umask(0000);
    mkfifo("/root/out0", 0777);
    mkfifo("/root/out1", 0777);

    char buffer = 0;

    int is = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    struct termios options;
    tcgetattr(is, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(is, TCIFLUSH);
    tcsetattr(is, TCSANOW, &options);

    int ofs0 = open("/root/out0", O_RDWR | O_NOCTTY | O_NDELAY);
    int ofs1 = open("/root/out1", O_RDWR | O_NOCTTY | O_NDELAY);

    while (!stopApplication)
    {
        char receivedBytes = read(is, &buffer, 1);

        if (receivedBytes != 0)
        {
            putchar(buffer);
            fflush(stdout);

            write(ofs0, &buffer, 1);

            write(ofs1, &buffer, 1);
        }

        //usleep(500000);
    }
    close(ofs0);
    close(ofs1);
    close(is);

    getchar();
    return 0;
}


# TtyCloner
Clone tty device to few pseudo ttys without worrying about blocking cloned device.

# Table of contents

* [General info](#general-info)
* [Example diagram](#example-diagram)
* [Build](#build)
* [Example of use](#example-of-use)
* [TODO](#todo)

# General info

You should run this application with root permissions or give permissions to needed devices/directories.
Console arguments:

    debian@debian10:~/TtyCloner/build$ ./TtyCloner
    Clone tty device to few pseudo ttys without worrying about blocking cloned device
    Usage:
      TtyCloner [OPTION...]
    
      -b, --baudrate arg   Set baudrate (default: 9600)
      -n, --number arg     Number of new pty interfaces
      -i, --interface arg  Tty interface to clone
      -f, --file           Create file /tmp/TtyCloner.txt with list of created
                           pty interfaces
      -h, --help           Show this help

# Example diagram

![Example diagram](https://raw.githubusercontent.com/lnarolski/TtyCloner/master/ExampleDiagram.png)

# Build

Commands to build on clean Debian 10 VM:

    sudo apt update
    sudo apt install git cmake g++ -y
    git clone https://github.com/lnarolski/TtyCloner
    cd TtyCloner
    mkdir build
    cd build
    cmake ..
    make

After compilation use TtyCloner binary file. You can also open and compile this project in Microsoft Visual Studio from TtyCloner.sln file.

# Example of use

<p align="center">
<a href="http://www.youtube.com/watch?feature=player_embedded&v=XCdNl2k3JyM" target="_blank"><img src="https://img.youtube.com/vi/XCdNl2k3JyM/0.jpg" 
alt="YouTube video" border="10" /></a>
</p>

# TODO
- Add list of created pty interfaces to shm
- Add interrupts

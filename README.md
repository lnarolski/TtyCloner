
# TtyCloner
Clone tty device to few pseudo ttys without worrying about blocking cloned device

# Table of contents

* [General info](#general-info)
* [Example diagram](#example-diagram)
* [Build](#build)
* [Example of use](#example-of-use)

# General info

You should run this application with root permissions or give permissions to needed devices/directories.

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

TODO

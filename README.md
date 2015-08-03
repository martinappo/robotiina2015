Robotiina
=========

Robotex 2014 participant 


##How to build:
###Under windows

* Install Visual Studo 2013 express
* Download OpenCV **2.4.9** and unpack into **c:\** drive 
 *  check that *C:\opencv\build\include* paths exist
 *  check that *C:\opencv\build\x86\vc12\lib* path exists
 *  check that *C:\opencv\build\x86\vc12\bin* path exists
* Download Boost binaries for VS2013 
 *  http://sourceforge.net/projects/boost/files/boost-binaries/1.56.0/ 
 *  **boost_1_56_0-msvc-12.0-32.exe**
 *  install into **C:\boost_1_56_0**
 *  check that *C:\boost_1_56_0\boost* path exists
 *  check that *C:\boost_1_56_0\lib32-msvc-12.0* path exists
* clone this project and compile it
* start robotiina.bat to launch it or launch it form IDE


###On Linux (Ubuntu/jessie)

* install cmake, gcc and git
 * sudo apt-get install build-essential cmake git 
* install  opencv 2.4.9, boost 1.55.0
 * sudo apt-get install libopencv-dev
 * sudo apt-get install libboost-all-dev
* clone repro
 * git clone https://github.com/martinappo/robotiina2015
 * cd robotiina2015
* run cmake .
* run make
* to start program execute ./robotiina

###On Rasperry Pi 2
* make sure you upgrade to jessie
 * Update /etc/apt/sources.list to have jessie wherever you've currently got wheezy
 * sudo apt-get update
 * sudo apt-get dist-upgrade
 * sudo rpi-update
 * reboot
* check that gcc version > 4.8
* Install OpenCV
 * http://www.pyimagesearch.com/2015/02/23/install-opencv-and-python-on-your-raspberry-pi-2-and-b/
* Install Boost 1.55.0
 * https://jeanleflambeur.wordpress.com/2014/06/07/compiling-boost-1-55-with-c11-support-on-the-raspberry-pi/comment-page-1/
 * Continue from "clone repro" on Linux instructions


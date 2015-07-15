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


###On Linux (Ubuntu)

* install cmake, gcc and git
 * sudo apt-get install build-essential cmake git 
* install  opencv 2.4.9, boost 1.55.0
 * sudo apt-get install libopencv-dev
 * sudo apt-get install libboost-all-dev
* clone repro
 * git clone https://github.com/andresviikmaa/Robotiina
 * cd robotiina
* run cmake .
* run make
* to start program execute ./robotiina



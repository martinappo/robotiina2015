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
* install ximea sdk
 * dowload sdk (XIMEA_Linux_SP.tgz) from http://www.ximea.com/support/documents/4
  * wget http://www.ximea.com/support/attachments/271/XIMEA_Linux_SP.tgz
  * tar -zxvf XIMEA_Linux_SP.tgz
  * cd package/
  * ./install -cam_usb30
* install  opencv 3.0.0
  * install gtk+2
    * sudo apt-get install libgtk2.0-dev
  * download latest opencv 3
    * wget https://github.com/Itseez/opencv/archive/3.0.0.zip
    * unzip 3.0.0.zip
    * cd opencv-3.0.0/
  * build using cmake
    * mkdir opencv-3.0.0
    * cd opencv-3.0.0/ 
    * cmake -D WITH_XIMEA=YES -D CMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
    * make
  * install using checkinstall
    * sudo apt-get install checkinstall
    * sudo checkinstall
* install boost >= 1.55.0
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
* follow linux instructions
* To use camera make sure /dev/video0 is present
 * "sudo modprobe bcm2835-v4l2" if driver not loaded


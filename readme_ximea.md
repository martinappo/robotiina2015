Ximea Camera Support

Compile OpenCV with Ximea 
download zip
unzip 3.0.0.zip
cd opencv-3.0.0/
mkdir opencv-3.0.0
cd opencv-3.0.0/
cmake -D WITH_XIMEA=YES -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/ ..
make
sudo checkinstall

# to remove pacage 
dpkg -r opencv

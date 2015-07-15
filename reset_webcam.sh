sudo ../usbreset `lsusb  | grep Webcam | sed -e "s/Bus \\([0-9]\+\\) Device \\([0-9]\+\\).*/\/dev\/bus\/usb\/\\1\/\2/"`


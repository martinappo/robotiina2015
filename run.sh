#!/bin/sh

v4l2-ctl -d /dev/video0 -c brightness=162
v4l2-ctl -d /dev/video0 -c contrast=162
v4l2-ctl -d /dev/video0 -c saturation=162
#v4l2-ctl -d /dev/video0 -c hue=-7
v4l2-ctl -d /dev/video0 -c white_balance_temperature_auto=0
v4l2-ctl -d /dev/video0 -c exposure_auto=1
v4l2-ctl -d /dev/video0 -c exposure_auto_priority=1
v4l2-ctl -d /dev/video0 -c exposure_absolute=95
v4l2-ctl -d /dev/video0 -c gain=44
v4l2-ctl -d /dev/video0 -c led1_mode=3
v4l2-ctl -d /dev/video0 -c led1_frequency=36
v4l2-ctl -d /dev/video0 -c focus_auto=0
v4l2-ctl -d /dev/video0 -c focus_absolute=5

v4l2-ctl -d /dev/video0 --set-fmt-video=width=640,height=480,pixelformat=0
v4l2-ctl -d /dev/video0 -p 60



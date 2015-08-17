FILES=videos/*
for f in $FILES
do
  if [ -d "$f" ]; then
  echo "Processing $f file..."

#	mencoder "mf://$f/*.jpg" -mf fps=25:type=jpg -ovc x264 -x264encopts bitrate=6000:threads=2 -o "$f.avi"
	ffmpeg -r 50 -start_number 1 -i "$f/%016d.jpg" "$f.mp4"
#	rm -rf "$f"
  fi
  # take action on each file. $f store current file name
  #cat $f
 done


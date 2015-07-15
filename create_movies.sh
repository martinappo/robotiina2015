FILES=videos/*
for f in $FILES
do
  echo "Processing $f file..."
  if [ -d "$f" ]; then
	mencoder "mf://$f/*.jpg" -mf fps=25:type=jpg -ovc x264 -x264encopts bitrate=6000:threads=2 -o "$f.avi"
	rm -rf "$f"
  fi
  # take action on each file. $f store current file name
  #cat $f
 done


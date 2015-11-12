#!/bin/sh
FILES=videos/*
for f in $FILES
do
  if [ -d "$f" ]; then
  echo "Processing $f file..."

	ls "$f/$*" | while read FILENAME
	do
	  FILENAME2=$(echo $FILENAME | sed -e s/^0000//g)
	  echo Renaming $FILENAME to $FILENAME2
	  mv "$f/$FILENAME" "$f/$FILENAME2"
#	  exit
	done
fi
done

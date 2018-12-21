#!/bin/bash

pictures=`find $1 -name "*.jpg"` #Searching for all pictures in directory and subdirectories
sorted_pictures=`ls -Rtr $pictures` #Sorting the pictures by modification time from oldest to newest
file_name_slashes=$1.jpg #Encoding the file name for the timeline
file_name=`echo $file_name_slashes | tr '/' '_'` #Replacing slashes with underscores
convert -append $sorted_pictures $file_name #Appending the sorted images into a timeline image

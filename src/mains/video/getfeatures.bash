#!/bin/bash
./trackfeatures --writefeatures=160_to_end.txt --numfeatures=50 --startframe=160 HAFB_overlap_bbox.vid 
./trackfeatures --writefeatures=160_to_beg.txt --readfeatures=160_to_end.txt --numfeatures=50 --startframe=160 HAFB_overlap_bbox.vid --endframe=0

lastfeature=`cat 160_to_end.txt | sort -k2 -n | awk 'END {print $2}'`

cp 160_to_end.txt features_2D_HAFB_overlap_bbox.txt

cat 160_to_beg.txt | sort -n -k2 | awk -v base=$lastfeature '{if ($2 >= 50) {print $1 "\t" $2 + base "\t" $3 "\t" $4 "\t" $5} else {print $0}}' >> features_2D_HAFB_overlap_bbox.txt

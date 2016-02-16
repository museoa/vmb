#! /usr/bin/bash

T=128
D=128
P=16
S=16
CS=16
OSCS=16

printf "\n"
printf "\t.org\t0x2000\n"
printf "\t.global PageTab\n"
printf "PageTab\tIS\t@\n\n"

printf "%%\tText Segment $T pages of 8kB\n"
for ((i=0; i<$T; i++ ))
do
  BASE=$((i*2))
  printf "\tOCTA 0x00000001%05X007+FirstUserPage\n" $BASE
done

printf "\n%%\tData Segment $D pages of 8kB\n"
printf "\tLOC\t(@&~0x1FFF)+0x2000\n"

for ((i=$T; i<$T+$D; i++ ))
do
  BASE=$((i*2))
  printf "\tOCTA 0x00000001%05X007+FirstUserPage\n" $BASE
done


printf "\n%%\tPool Segment $P pages of 8kB\n"
printf "\tLOC\t(@&~0x1FFF)+0x2000\n"

for ((i=$T+$D; i<$T+$D+$P; i++ ))
do
  BASE=$((i*2))
  printf "\tOCTA 0x00000001%05X007+FirstUserPage\n" $BASE
done


printf "\n%%\tStack Segment $S pages of 8kB\n"
printf "\tLOC\t(@&~0x1FFF)+0x2000\n"

for ((i=$T+$D+$P; i<$T+$D+$P+$S; i++ ))
do
  BASE=$((i*2))
  printf "\tOCTA 0x00000001%05X007+FirstUserPage\n" $BASE
done

printf "\n%%\tStack Segment $CS pages of 8kB for the gcc Stack\n"
printf "\tLOC\t(@&~0x1FFF)+0x2000-$CS*8\n"

for ((i=$T+$D+$P+$S; i<$T+$D+$P+$S+$CS; i++ ))
do
  BASE=$((i*2))
  printf "\tOCTA 0x00000001%05X007+FirstUserPage\n" $BASE
done

printf "\n\nUserRamSize\tIS\t0x%X" $((($T+$D+$P+$S+$CS)*0x2000))
printf "\t%% size of memory allocated for user programs\n"



#!/bin/sh

ffmpeg -i this_is_test.wav -f s16le -acodec pcm_s16le this_is_test.pcm
echo -n "const " > this_is_test.h
xxd -i this_is_test.pcm >> this_is_test.h

echo "done!"

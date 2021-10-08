#!/bin/sh
/channels/tnfsd/tnfsd /channels/tnfsd &
P1=$!
/channels/hub/bin/channels_hub & # your second application
P2=$!
wait $P1 $P2

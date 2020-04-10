#!/bin/sh
TAG="0.2.2"
docker build . --no-cache -t voip_patrol
docker tag voip_patrol:latest jchavanton/voip_patrol:latest
docker tag voip_patrol:latest jchavanton/voip_patrol:${TAG}
# docker push jchavanton/voip_patrol:latest
# docker push jchavanton/voip_patrol:${TAG}

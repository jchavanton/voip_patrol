#!/bin/sh
TAG="0.2.1"
docker build . --no-cache -t voip_patrol
docker tag voip_patrol:latest jchavanton/voip_patrol:latest
docker tag voip_patrol:latest jchavanton/voip_patrol:${TAG}
# docker push voip_patrol:latest
# docker push flowroute/voip_patrol:${TAG}

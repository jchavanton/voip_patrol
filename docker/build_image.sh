#!/bin/sh
TAG="0.8.1"
docker build . --no-cache --network=host -t voip_patrol
docker tag voip_patrol:latest jchavanton/voip_patrol:latest
docker tag voip_patrol:latest jchavanton/voip_patrol:${TAG}
echo "Don't forget to push !"
echo "docker push jchavanton/voip_patrol:latest && docker push jchavanton/voip_patrol:${TAG}"

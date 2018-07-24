#!/bin/sh

DIR_PREFIX=`pwd`
echo $DIR_PREFIX
IMAGE=voip_patrol:latest
docker stop voip_patrol
docker rm voip_patrol

docker run -d \
	--name=voip_patrol \
	--volume $DIR_PREFIX/../xml:/xml \
	$IMAGE \
	./git/voip_patrol/voip_patrol --port 5555 --conf /xml/tls_server.xml -l /log/voip_patrol.log --output result.json
#	/bin/sh -c "tail -f /dev/null"

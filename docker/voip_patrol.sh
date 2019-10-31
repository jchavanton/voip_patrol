#!/bin/sh
DIR_PREFIX=`pwd`
IMAGE=voip_patrol:latest
docker stop voip_patrol
docker rm voip_patrol
docker run -d --net=host --name=voip_patrol --volume $DIR_PREFIX/../xml:/xml \
	$IMAGE \
	./git/voip_patrol/voip_patrol -p 5555 -c /xml/tls_server.xml -l /log/voip_patrol.log -o result.json
#	/bin/sh -c "tail -f /dev/null"

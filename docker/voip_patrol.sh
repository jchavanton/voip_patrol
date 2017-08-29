#!/bin/sh

# IMAGE=gliderlabs/alpine:3.6
DIR_PREFIX="/tmp/voip_patrol"
SCENARIO_FILE="1.xml"
RESULT_JSON_FILE="result.json"
IMAGE=voip_patrol:0.2
mkdir -p ${DIR_PREFIX}/logs
mkdir -p ${DIR_PREFIX}/scenarios
[ -e ${DIR_PREFIX}/logs/${RESULT_JSON_FILE} ] && rm ${DIR_PREFIX}/logs/${RESULT_JSON_FILE}
cp ${SCENARIO_FILE} ${DIR_PREFIX}/scenarios
docker stop voip_patrol
docker rm voip_patrol
# docker pull $IMAGE

docker run -d \
	--name=voip_patrol \
	-v ${DIR_PREFIX}:/vp \
	$IMAGE \
	./voip_patrol/voip_patrol -c /vp/scenarios/1.xml -l /vp/logs/voip_patrol.log -o /vp/logs/result.json
#	/bin/sh -c "while true; do sleep 2; echo date; done"

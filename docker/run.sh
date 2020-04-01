#!/bin/sh
DIR_PREFIX=`pwd`
IMAGE=voip_patrol:latest
CONTAINER_NAME=voip_patrol
docker rm ${CONTAINER_NAME}
docker run -d --net=host --name=${CONTAINER_NAME} ${IMAGE}


# PORT=5060
# XML_CONF="basic_server.xml"
# RESULT_FILE="result.json"
#
# docker run -d --net=host \
#     --name=${CONTAINER_NAME} \
#     --env XML_CONF=`echo ${XML_CONF}` \
#     --env PORT=`echo ${PORT}` \
#     --env RESULT_FILE=`echo ${RESULT_FILE}` \
#     --env EDGEPROXY=`echo $EDGEPROXY` \
#     --volume $DIR_PREFIX/xml:/xml \
#     --volume $DIR_PREFIX/log:/output \
#     --volume $DIR_PREFIX/voice_files:/voice_ref_files \
#     ${IMAGE}

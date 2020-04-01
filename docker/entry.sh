#!/bin/bash

XML_CONF=${XML_CONF-"basic_server.xml"}
RESULT_FILE=${RESULT_FILE-"result.json"}
PORT=${PORT="5060"}

echo "XML_CONF[${XML_CONF}] RESULT FILE[${RESULT_FILE}] PORT[$PORT]"

if [ "$1" = "" ]; then
	CMD="/git/voip_patrol/voip_patrol --port ${PORT} --conf /xml/${XML_CONF} --output /output/${RESULT_FILE}"
else
	CMD="$*"
fi

echo "Running ${CMD}"
exec ${CMD}

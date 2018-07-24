
FROM alpine:3.8

RUN echo "building VoIP Patrol" \
	&& apk update && apk add git cmake g++ cmake make curl-dev alsa-lib-dev \
	&& mkdir /git && cd /git && git clone https://github.com/jchavanton/voip_patrol.git \
	&& cd voip_patrol && git checkout master \
	&& git submodule update --init \
	&& cp include/config_site.h  pjsua/pjlib/include/pj/config_site.h \
	&& cd pjsua && ./configure && make dep && make && make install \
	&& cd .. && cmake CMakeLists.txt && make

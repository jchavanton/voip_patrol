FROM debian

ARG VERSION="v0.12"
RUN echo "building VoIP Patrol" && apt-get update \
	&& apt-get install -y build-essential libcurl4-openssl-dev cmake pkg-config libasound2-dev \
	&& apt-get -y install libssl-dev git \
	&& mkdir /git && cd /git && git clone https://github.com/jchavanton/voip_patrol.git \
	&& cd voip_patrol && git checkout ${VERSION} \
	&& git submodule update --init \
	&& cp include/config_site.h  pjproject/pjlib/include/pj/config_site.h \
	&& cd pjproject && ./configure --disable-libwebrtc && make dep && make && make install \
	&& cd .. && cmake CMakeLists.txt && make

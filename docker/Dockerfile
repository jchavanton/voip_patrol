FROM python:3.7-buster

ARG VERSION="0.7.8"
ARG REPO="https://github.com/jchavanton/voip_patrol.git"

RUN echo "installing dependencies" \
	&& apt-get update && apt-get install -y build-essential libcurl4-openssl-dev cmake pkg-config libasound2-dev libopus0 libopus-dev
	&& apt-get -y install libssl-dev git

RUN echo "building VoIP Patrol" \
	&& mkdir /git && cd /git && git clone ${REPO} \
	&& cd voip_patrol && git checkout ${VERSION} \
	&& git submodule update --init \
	&& cp include/config_site.h  pjproject/pjlib/include/pj/config_site.h \
	&& cd pjproject && ./configure --disable-libwebrtc --disable-opencore-amr \
	&& make dep && make && make install \
	&& cd .. && cmake CMakeLists.txt && make

RUN ln -s /git/voip_patrol/voice_ref_files /voice_ref_files

RUN mkdir /xml
RUN mkdir /output

COPY xml/basic_server.xml /xml
COPY entry.sh /
ENTRYPOINT ["/entry.sh"]

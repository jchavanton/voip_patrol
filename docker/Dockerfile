# Two-stage production image for voip_patrol.
#
# Stage 1 (builder): full toolchain, compiles pjproject + voip_patrol
# from the source tree COPY'd in. Same recipe as tests/Dockerfile.test.
#
# Stage 2 (runtime): debian:trixie-slim with just the shared libraries
# the binary links against. Binary goes to /vp/voip_patrol, voice
# samples to /vp/voice_ref_files, entry script at /entry.sh.
#
# Version string:
#   Pass --build-arg VP_VERSION=<tag> so the binary reports the right
#   version. CI passes the pushed git tag. Without it, CMake falls
#   back to `git describe` (inside the builder stage, which has the
#   .git tree from COPY) and then to "dev".
#
# Build (from repo root):
#   docker build -f docker/Dockerfile --build-arg VP_VERSION=$(git describe --tags --always) -t voip_patrol .

# ---- Stage 1: build ---------------------------------------------------------
FROM debian:trixie AS builder

ARG VP_VERSION=""

RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		build-essential \
		cmake \
		pkg-config \
		git \
		libcurl4-openssl-dev \
		libasound2-dev \
		libopus-dev \
		libssl-dev \
		uuid-dev \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . /build

RUN cp include/config_site.h pjproject/pjlib/include/pj/config_site.h \
	&& find pjproject -name "*.depend" -delete \
	&& cd pjproject \
	&& ./configure --disable-libwebrtc --disable-opencore-amr \
	&& make dep && make && make install \
	&& cd /build \
	&& cmake -DVP_VERSION="${VP_VERSION}" CMakeLists.txt \
	&& make -j"$(nproc)" \
	&& strip voip_patrol

# ---- Stage 2: runtime -------------------------------------------------------
FROM debian:trixie-slim

RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		libcurl4 \
		libasound2 \
		libopus0 \
		libssl3 \
	&& rm -rf /var/lib/apt/lists/*

RUN mkdir -p /vp /xml /output
COPY --from=builder /build/voip_patrol      /vp/voip_patrol
COPY --from=builder /build/voice_ref_files  /vp/voice_ref_files

# /voice_ref_files is the stable path XML configs reference (see
# docker/xml/basic_server.xml). Keep the symlink so existing user
# configs don't need changes.
RUN ln -s /vp/voice_ref_files /voice_ref_files

COPY docker/xml/basic_server.xml /xml/
COPY docker/entry.sh /entry.sh

WORKDIR /vp
ENTRYPOINT ["/entry.sh"]

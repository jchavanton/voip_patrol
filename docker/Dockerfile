
FROM alpine:3.8

ENV DEB_PKG="netcat curl dnsutils sqlite net-tools vim ngrep wget libhiredis-dev"
ENV DEB_PKG_TMP="bison flex build-essential git libssl-dev libpq-dev liblua5.1-0-dev libcurl4-openssl-dev libncurses5-dev libxml2-dev libsqlite3-dev"
ENV DEB_PKG_DEP="libcurl3 libssl1.1 libpq5 liblua5.1-0 libncurses5 libxml2 libsqlite3-0"

RUN echo "building VoIP Patrol" \
	&& apk update && apk add git cmake g++ cmake make curl-dev alsa-lib-dev \
	&& mkdir /git && cd /git && git clone https://github.com/jchavanton/voip_patrol.git && cd voip_patrol && git checkout master \
	&& git submodule update --init \
	&& cp include/config_site.h  pjsua/pjlib/include/pj/config_site.h \
	&& cd pjsua && ./configure && make dep && make && make install \
	&& cd .. && cmake CMakeLists.txt && make

# VOLUME ["/acc_logs"]
# VOLUME ["/kamailio_db"]
# VOLUME ["/lrn_db"]

# # Standard SIP ports.
# EXPOSE 5060 5060/udp
# EXPOSE 5060 5060/tcp
# # TLS/SIPS ports.
# EXPOSE 5061 5061/tcp
# # ALG-bypassing alternate ports. (primarily for natproxy)
# EXPOSE 5160 5160/udp
# # XMLRPC port (natproxy Dialog cutting)
# EXPOSE 4192
# 
# # is this health check actually used for anything besides the compose file?
# # If no, we may want to remove it.
# # This currently spams datadog with bogus healthcheck events at NORMAL level
# # we can run with --no-healthcheck to disable on a case-by-case basis
# # newer datadogs SHOULD log these as low priority which partly cuts down on noise
# # see https://github.com/DataDog/dd-agent/pull/2960
# # do we need to update our DD container?
# HEALTHCHECK --interval=10s --timeout=1s CMD curl -f http://localhost:5059/ping
# 
# # Add Tini
# ENV TINI_VERSION v0.17.0
# ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /tini
# RUN chmod +x /tini
# ENTRYPOINT ["/tini", "--", "/entry.sh"]
# 
# # These are ordered in increasing order of likelihood to change, to make the
# # build cycle for development faster.
# COPY schemas/ /schemas/
# 
# COPY deploy/compose /deploy/compose
# COPY deploy/secrets /deploy/secrets
# COPY deploy/unpack_secrets.sh /deploy/unpack_secrets.sh
# COPY deploy/consul.tmpl /deploy/consul.tmpl
# 
# COPY scripts/ /scripts/
# COPY entry.sh /
# COPY config/ /etc/kamailio/
# 
# ARG BUILD_REF=local
# ARG BUILD_DATE=unknown
# LABEL	org.label-schema.name="kamailio" \
# 	org.label-schema.vendor="Flowroute" \
# 	org.label-schema.description="Base Debian Kamailio image for Flowroute SIP stack" \
# 	org.label-schema.url="http://gitlab.internal/dev/sip-elements/README.md" \
# 	org.label-schema.vcs-url="http://gitlab.internal/dev/sip-elements" \
# 	org.label-schema.vcs-ref="${BUILD_REF}" \
# 	org.label-schema.build-date="${BUILD_DATE}" \
# 	org.label-schema.version="1.8.14" \
# 	org.label-schema.schema-version="1.0" \
# 	org.label-schema.docker.cmd="" \
# 	org.label-schema.docker.cmd.test="docker run [container] smoke_test.sh" \
# 	org.label-schema.docker.cmd.debug="docker exec -it [container] sh" \
# 	org.label-schema.docker.params="SIP_ELEMENT=REGOUT element to run,SIP_ELEMENT_IP=10.0.0.1 address to advertise,SERVICE_NAME=regout consul service to advertise,DB_HOST=foo.rds.amazon.com Host name of DB,DB_DATABASE=flowroute database name to connect to,DB_PASSWORD= password to use for DB,DB_USER=flowroute username to connect to DB,SHARED_MEM=512 in megabytes,PRIVATE_MEM=128 in megabytes"

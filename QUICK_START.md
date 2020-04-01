## Quick start with docker

### cloning voip_patrol
(only needed to get the shell scripts to build your own image)
```
git clone https://github.com/jchavanton/voip_patrol.git
```

### example: building docker image
(optional, you can pull unless you want to build your own image)
```
cd voip_patrol/docker
./build_image.sh
```

### example: pulling image from dockerhub
```
docker pull jchavanton/voip_patrol:latest
```

### example: running the container
```
cd voip_patrol/docker
./run.sh
```
#### or use the following
```bash
#!/bin/sh
IMAGE=voip_patrol:latest
CONTAINER_NAME=voip_patrol
docker run -d --net=host --name=${CONTAINER_NAME} ${IMAGE}
```

### done, voip_patrol should running as a server on 5060 !
```
docker logs voip_patrol
XML_CONF[basic_server.xml] RESULT FILE[result.json] PORT[5060]
Running /git/voip_patrol/voip_patrol --port 5060 --conf /xml/basic_server.xml --output /output/result.json
```

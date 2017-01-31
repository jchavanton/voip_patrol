# VoIP Patrol

## VoIP signaling and media test automaton

### debian dependencies
```
 apt-get install build-essential
 apt-get install libcurl4-openssl-dev
 apt-get install cmake
 apt-get install pkg-config
 apt-get install libasound2-dev
```
### build pjsua
```
 git submodule update --init
 cd pjsua && ./configure
 make dep && make
 cd ..
```

### buil voip_patrol
```
 cmake CMakeLists.txt 
 make
```

### run
```
 ./voip_patrol
 -c,--conf <conf.xml> 
 -l,--log <test.log>
```
### sources
```
src/
├── curl
│   ├── email.c
│   └── email.h
├── ezxml
│   ├── changelog.txt
│   ├── ezxml.c
│   ├── ezxml.h
│   ├── ezxml.html
│   ├── ezxml.txt
│   └── license.txt
├── PESQ
│   ├── dsp.c
│   ├── dsp.h
│   ├── dsp.o
│   ├── Makefile
│   ├── pesqdsp.c
│   ├── pesqdsp.o
│   ├── pesq.h
│   ├── pesqio.c
│   ├── pesqio.o
│   ├── pesqmain.c
│   ├── pesqmain.o
│   ├── pesqmod.c
│   ├── pesqmod.o
│   ├── pesqpar.h
│   ├── pesq_results.txt
│   └── run_pesq
└── voip_patrol
    ├── patrol_pesq.c
    ├── patrol_pesq.h
    ├── voip_patrol.cpp
    └── voip_patrol.hh
```

#### congfig test plan example
```xml
<?xml version="1.0"?>
<config>
	<actions>
		<action type="register" expected_cause_code="200" username="666" password="DNV9J1xUU" registrar="sip.server.net"/>
		<action type="register" expected_cause_code="200" username="777" password="SMOF0qFRA" registrar="sip.server.net"/>
		<action type="register" expected_cause_code="403" username="555" password="SMOF0qFRA" registrar="sip.server.net"/>
		<action type="register" expected_cause_code="204" username="555" password="SMOF0qFRA" registrar="sip.server.net"/>
		<action type="wait"/>
		<action type="call" expected_cause_code="404" caller="666@sip.server.net" callee="555@sip.server.net" duration=10/>
		<action type="call" expected_cause_code="200" caller="666@sip.server.net" callee="777@sip.server.net" duration=10/>
	</actions>
</config>
```
#### PJSUA
PJSUA : A C++ High Level Softphone API : built on top of PJSIP and PJMEDIA
http://www.pjsip.org

#### PESQ
P.862 : Perceptual evaluation of speech quality (PESQ): An objective method for end-to-end speech quality assessment of narrow-band telephone networks and speech codecs
http://www.itu.int/rec/T-REC-P.862
```
./run_pesq +16000 voice_files/reference.wav voice_files/recording.wav
```

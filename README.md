# VoIP Patrol

## VoIP signaling and media test automaton

### mac dependencies 
(mac is not supported ! cmake is currently missing portaudio lib detection and linking)
```
 brew install pkg-config
 brew install cmake
```

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
 make dep && make && make install
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

### scenario actions
`register` create a test account `account1` and will register to `1.1.1.1`
```xml
<action type="register"
 expected_cause_code="200"
 username="account1"
 password="password"
 realm="sip.domain.com" 
 registrar="1.1.1.1"/>
```
`wait` wait for the tests in progress to complete
```xml
<action type="wait"/>
```
`accept` when receiving an inbound call on the specified registered callee, create a test with the specified parameters  
```xml
<action type="accept"
 label="my_label_-1"
 callee="33663170"
 hangup="2"/>
```
`call` make a call using the specified callee and create a test with the specified parameters
```xml
<action type="call"
 label="my_label_-1"
 expected_cause_code="200"
 caller="33663170@111.75.65.193"
 callee="12062092868@111.75.65.194"
 max_duration="4"/>
```
`alert` send an email with the test results
```xml
<action type="alert"
 email="your_email@gmail.com"
 email_from="test@voip-patrol.org"
 smtp_host="smtp://gmail-smtp-in.l.google.com:25"/>
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
include/
└── log.h
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

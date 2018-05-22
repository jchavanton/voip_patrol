# VoIP Patrol

## VoIP signaling and media test automaton

### Linux Alpine dependencies
```
apk update
apk add git cmake g++ cmake make openssl-dev curl-dev alsa-lib-dev
```
### Linux Debian dependencies
```
apt-get install build-essential libcurl4-openssl-dev cmake pkg-config libasound2-dev
```
TLS
```
apt-get install libssl-dev
```
### build pjsua
```
git clone https://github.com/jchavanton/voip_patrol.git
cd voip_patrol && git submodule update --init
cp include/config_site.h  pjsua/pjlib/include/pj/config_site.h # increase the max amount of account and calls
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
  -v --vesion                       voip_patrol version
  --log-level-file <0-10>           file log level
  --log-level-console <0-10>        console log level
  -p --port <5060>                  local port
  -c,--conf <conf.xml>              XML scenario file
  -l,--log <logfilename>            voip_patrol log file name
  -o,--output <result.json>         json result file name
```

### scenario actions
`register` create a test account `account1` and will register to `1.1.1.1`
```xml
<action type="register"
   expected_cause_code="200"
   transport="udp"
   username="account1"
   password="password"
   realm="sip.domain.com"
   registrar="1.1.1.1"
/>
```
`wait` wait for the tests in progress to complete or until they reach their wait_until state
```xml
<action type="wait"/>
```

`wait-complete` wait for tests in progress to complete
```xml
<action type="wait-complete"/>
```

`accept` when receiving an inbound call on the specified account, create a test with the specified parameters
```xml
<action type="accept"
   label="my_label_-1"
   account="63633170"
   hangup="2"
   max_duration="10"
   wait_until="2"
/>
```
`call` make a call using the specified callee and create a test with the specified parameters
```xml
<action type="call"
   label="call_with_creds"
   wait_until="3"
   expected_cause_code="200"
   callee="+12061234567@2.2.2.2"
   caller="68595971@1.1.1.1"
   username="68595971"
   password="xxxxxx"
   realm="sip.domain.com"
   max_duration="4"
   hangup="10"
/>
```

`alert` send an email with the test results
```xml
<action type="alert"
   email="your_email@gmail.com"
   email_from="test@voip-patrol.org"
   smtp_host="smtp://gmail-smtp-in.l.google.com:25"
/>
```

## Dependencies 

#### PJSUA
PJSUA : A C++ High Level Softphone API : built on top of PJSIP and PJMEDIA
http://www.pjsip.org

#### PJSUA2
http://www.pjsip.org/docs/book-latest/PJSUA2Doc.pdf

#### PESQ
P.862 : Perceptual evaluation of speech quality (PESQ): An objective method for end-to-end speech quality assessment of narrow-band telephone networks and speech codecs
http://www.itu.int/rec/T-REC-P.862
```
./run_pesq +16000 voice_files/reference.wav voice_files/recording.wav
```

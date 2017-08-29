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
### build pjsua
```
git clone https://github.com/jchavanton/voip_patrol.git
cd voip_patrol && git submodule update --init
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
 -c,--conf conf.xml 
 -l,--log <logfilename> 
 -o,--output results.json
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

## Dependencies 

#### PJSUA
PJSUA : A C++ High Level Softphone API : built on top of PJSIP and PJMEDIA
http://www.pjsip.org

#### PESQ
P.862 : Perceptual evaluation of speech quality (PESQ): An objective method for end-to-end speech quality assessment of narrow-band telephone networks and speech codecs
http://www.itu.int/rec/T-REC-P.862
```
./run_pesq +16000 voice_files/reference.wav voice_files/recording.wav
```

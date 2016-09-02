# VoIP Test

## VoIP signaling and media tester

### debian dependencies
```
 apt-get install build-essential
 apt-get install libcurl4-openssl-dev
 apt-get install cmake
```
### build
```
 cd pjsua && ./configure && ./make
 cd .. && cmake && make
```
### run
```
 ./voip_hyppo
 -c,--conf <conf.xml> 
 -l,--log <test.log>
```
### sources
```
 pjsip-apps/src/samples/voip_hyppo/voip_hyppo.cpp
 pjsip-apps/src/samples/voip_hyppo/voip_hyppo.hh
```

#### xml parser integration
```
 pjsip-apps/src/samples/voip_hyppo/ezxml
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

#### PESQ
```
./run_pesq +16000 voice_files/reference.wav voice_files/recording.wav
```

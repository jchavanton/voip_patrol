[![Docker Pulls](https://img.shields.io/docker/pulls/jchavanton/voip_patrol.svg)](https://hub.docker.com/r/jchavanton/voip_patrol/)

# VoIP Patrol
![GitHub Logo](VP_Logo_1200px-11th_Airborne_Division.patch_small2.jpg)

## VoIP signaling and media test automaton
Designed to automate end2end and or integration tests.

VoIP patrol will follow a scenario in XML format and will output results in JSON.

Each line in the output file is a separate JSON structure, note that the entire file is not a valid JSON file,
this is because VoIP patrol will output results as they become available.

It is possible to test many scenarios that are not easy to test manually like a re-invite with a new codec.

### Docker quick start
[quick start with docker](QUICK_START.md)


### Linux Debian building from sources
[see commands in Dockerfile](docker/Dockerfile)

### Load test example
[load test example](load_test/LOAD_TEST.md)

### run
```
./voip_patrol --help
```

### Example: making a test call
```xml
<config>
  <actions>
    <action type="call" label="us-east-va"
            transport="tls"
            expected_cause_code="200"
            caller="15147371787@noreply.com"
            callee="12012665228@target.com"
            to_uri="+12012665228@target.com"
            max_duration="20" hangup="16"
            username="VP_ENV_USERNAME"
            password="VP_ENV_PASSWORD"
            rtp_stats="true"
    >
        <x-header name="X-Foo" value="Bar"/>
    </action>
    <!-- note: param value starting with VP_ENV_ will be replaced by environment variables -->
    <!-- note: rtp_stats will include RTP transmission statistics -->
    <!-- note: x-header tag inside an action will append an header -->
    <action type="wait" complete="true"/>
  </actions>
</config>
```
### Sample JSON output
```json
{
  "2": {
    "label": "us-east-va",
    "start": "17-07-2018 00:00:05",
    "end": "17-07-2018 00:00:24",
    "action": "call",
    "from": "15147371787",
    "to": "12012665228",
    "result": "PASS",
    "expected_cause_code": 200,
    "cause_code": 200,
    "reason": "Normal call clearing",
    "callid": "7iYDFukJr-9BOLOmWg.7fZyHZeZUAwao",
    "transport": "TLS",
    "peer_socket": "34.226.136.32:5061",
    "duration": 16,
    "expected_duration": 0,
    "max_duration": 20,
    "hangup_duration": 16,
    "rtp_stats_0": {
      "rtt": 0,
      "remote_rtp_socket": "10.250.7.88:4028",
      "codec_name": "PCMU",
      "clock_rate": "8000",
      "Tx": {
        "jitter_avg": 0,
        "jitter_max": 0,
        "pkt": 816,
        "kbytes": 127,
        "loss": 0,
        "discard": 0,
        "mos_lq": 4.5
      },
      "Rx": {
        "jitter_avg": 0,
        "jitter_max": 0,
        "pkt": 813,
        "kbytes": 127,
        "loss": 0,
        "discard": 0,
        "mos_lq": 4.5
      }
    }
  }
}
```

### Example: starting a TLS server
```bash
./voip_patrol \
   --port 5060 \ # TLS port 5061 +1
   --conf "xml/tls_server.xml" \
   --tls-calist "tls/ca_list.pem" \
   --tls-privkey "tls/key.pem" \
   --tls-cert "tls/certificate.pem" \
   --tls-verify-server \
```

```xml
<config>
  <actions>
     <!-- note: default is the "catch all" account,
          else account as to match called number -->
    <action type="accept"
            account="default"
            hangup="5"
            play_dtmf="0123456789#*"
            play="voice_ref_files/f.wav"
            code="200" reason="YES"
            ring_duration="5"
    />
    <!-- DTMF will be sent using RFC2833 -->
    <!-- note: wait for new incoming calls
               forever and generate test results -->
    <action type="wait" ms="-1"/>
  </actions>
</config>
```

### Example: accepting calls and checking for specific header
```xml
<config>
  <actions>
    <action type="accept"
            account="default"
            hangup="5"
            code="200" reason="OK"
    >
        <check-header name="Min-SE"/>
        <check-header name="X-Foo" value="Bar"/>
    </action>
    <action type="wait" ms="-1"/>
  </actions>
</config>
```

### Example: accepting calls and checking for specific header with exact match or regular expression and no match on other
```xml
<config>
  <actions>
    <action type="accept"
            match_account="default"
            hangup="5"
            code="200" reason="OK"
    >
        <check-header name="Min-SE"/>
        <!-- Chech that a header exists -->
        <check-header name="X-Foo" value="Bar"/>
        <!-- Chech that a header exists and have a specific value -->
        <check-header name="From" regex="^.*sip:\+1234@example\.com"/>
        <!-- Chech that a header exists and matches a specific regex -->
        <check-header name="To" regex="^.*sip:\+5678@example\.com" fail_on_match="true"/>
        <!-- Chech that a header exists and NOT matches a specific regex -->
        <check-header name="RURI" regex="^INVITE\ sip:\d{5}@(\d{1,3}\.){3}\d{1,3}:\d{1,5};.*transport=[a-zA-Z]{3};.*"/>
        <!-- Not really a header, but allows to check the Request URI on an incoming INVITE-->
    </action>
    <action type="wait" ms="-1"/>
  </actions>
</config>
```

### Example: accepting calls and searching the message with a regular expression
```xml
<config>
  <actions>
    <action type="accept"
            account="default"
            hangup="5"
            code="200" reason="OK"
    >
        <check-message method="INVITE" regex="m=audio(.*)RTP/AVP 0 8.*"/>
        <!-- searching for pcmu pcma in the SDP -->
    </action>
    <action type="wait" ms="-1"/>
  </actions>
</config>
```

### Example: making tests calls with wait_until
Scenario execution is sequential and non-blocking.
We can use “wait” command with previously set “wait_until” params
to control parallel execution.

```
Call States
NULL : Before INVITE is sent or received
CALLING : After INVITE is sent
INCOMING : After INVITE is received.
EARLY : After response with To tag.
CONNECTING : After 2xx is sent/received.
CONFIRMED : After ACK is sent/received.
DISCONNECTED
```
```xml
<config>
  <actions>
    <action type="call" label="call#1"
            transport="udp"
            wait_until="CONFIRMED"
            expected_cause_code="200"
            caller="15148888888@noreply.com"
            callee="12011111111@target.com"
    />
    <!-- note: will wait until all tests pass wait_until state -->
    <action type="wait"/>
    <action type="call" label="call#2"
            transport="udp"
            wait_until="CONFIRMED"
            expected_cause_code="200"
            caller="15147777777@noreply.com"
            callee="12012222222@target.com"
    />
    <action type="wait" complete="true"/>
  </actions>
</config>
```

### Example: testing registration
```xml
<config>
  <actions>
    <!-- note: proxy param to send to a proxy -->
    <action type="register" label="register target.com"
            transport="udp"
            account="VP_ENV_USERNAME"
            username="VP_ENV_USERNAME"
            password="VP_ENV_PASSWORD"
            proxy="172.16.7.1"
            registrar="target.com"
            expected_cause_code="200"
    />
    <action type="wait" complete="true"/>
  </actions>
</config>
```

### Example: re-invite with new codec
```xml
<config>
    <action>
        <action type="codec" disable="all"/>
        <action type="codec" enable="pcma" priority="250"/>
        <action type="codec" enable="pcmu" priority="248"/>

        <!-- call that will last 12 seconds and re-invite every 2 seconds -->
        <action type="call"
            wait_until="CONFIRMED"
            expected_cause_code="200"
            caller="16364990640@125.22.198.115"
            callee="12349099229@sip.mydomain.com"
            max_duration="55" hangup="12"
            username="65454659288" password="adaadzWidD7T"
            re_invite_interval="2"
            rtp_stats="true"
        />
        <action type="wait"/> <!-- this will wait until the call is confirmed -->
        <action type="codec" disable="pcma"/>
        <!-- re-invite will now use pcmu forcing a new session -->
        <action type="wait" ms="3000"/> <!-- this will wait 3 seconds -->
        <action type="codec" enable="pcma" priority="250"/>
        <!-- re-invite will now use pcma forcing a new session -->

        <action type="wait" complete="true"> <!-- Wait until the calls are disconnected -->
    <actions/>
<config/>
```

### Example: Overwriting local contact header
```xml
<config><actions>
    <action type="codec" disable="all"/>
    <action type="codec" enable="pcma" priority="250"/>
    <action type="codec" enable="gsm" priority="249"/>
    <action type="codec" enable="pcmu" priority="248"/>

    <action type="call"
        transport="udp"
        caller="+15147371787@fakecustomer.xyz"
        callee="+911@edgeproxy1"
        transport="udp"
        username="20255655"
        password="qntzhpbl"
        rtp_stats="true"
        late_start="false"
        force_contact="sip:+15147371787@10.10.2.5:5777"
        play="/git/voip_patrol/voice_ref_files/reference_8000_12s.wav"
        hangup="5">

    <x-header name="Foo" value="Bar"/>
    </action>
    <action type="wait" complete="true" />
</actions></config>
```

### Example: WAIT action
#### wait forever:
```xml
<action type="wait" ms="-1"/>
```
#### wait until you receive a certain amount of calls
```xml
<action type="accept" call_count="x" ... />
<action type="wait" complete="true"/>
```
#### wait 5 seconds or one call
```xml
<action type="accept" call_count="1" ... />
<action type="wait" ms="5000"/>
```

### Sample JSON output RTP stats report with multiples sessions
#### one block is generated everytime a session is created
```json
{
 "rtp_stats_0": {
      "rtt": 0,
      "remote_rtp_socket": "10.250.7.88:4028",
      "codec_name": "PCMA",
      "clock_rate": "8000",
      "Tx": {
        "jitter_avg": 0,
        "jitter_max": 0,
        "pkt": 105,
        "kbytes": 16,
        "loss": 0,
        "discard": 0,
        "mos_lq": 4.5
      },
      "Rx": {
        "jitter_avg": 0,
        "jitter_max": 0,
        "pkt": 104,
        "kbytes": 16,
        "loss": 0,
        "discard": 0,
        "mos_lq": 4.5
      }
    },
    "rtp_stats_1": {
      "rtt": 0,
      "remote_rtp_socket": "10.250.7.89:40230",
      "codec_name": "PCMU",
      "clock_rate": "8000",
      "Tx": {
        "jitter_avg": 0,
        "jitter_max": 0,
        "pkt": 501,
        "kbytes": 78,
        "loss": 0,
        "discard": 0,
        "mos_lq": 4.5
      },
      "Rx": {
        "jitter_avg": 0,
        "jitter_max": 0,
        "pkt": 501,
        "kbytes": 78,
        "loss": 0,
        "discard": 0,
        "mos_lq": 4.5
      }
    }
}
```
### Example: email reporting
```xml
<config>
  <actions>
    <action type="alert"
     email="jchavanton+vp@gmail.com"
     email_from="test@voip-patrol.org"
     smtp_host="smtp://gmail-smtp-in.l.google.com:25"
    />
    <!-- add more test actions here ...  -->
    <action type="wait" complete="true"/>
  </actions>
</config>
```

### accept command parameters

| Name | Type | Description |
| ---- | ---- | ----------- |
| ring_duration | int | ringing duration in seconds |
| early_media | bool | if "true" 183 with SDP and early media is used |
| timer | string | control SIP session timers, possible values are : inactive, optional, required or always |
| code | int | SIP cause code to return must be >100 and <700 |
| account | string | Account will be used if it matches the user part of an incoming call RURI or "default" will catch all |
| response_delay | int | ms delay before reponse is sent, useful to test timeouts and race conditions |
| call_count | int | The amount of calls to receive to consider the command completed, default -1 (considered completed) |
| transport | string | Force a specific transport for all messages on accepted calls, default to all transport available |
| re_invite_interval | int | Interval in seconds at which a re-invite with SDP will be sent |
| rtp_stats | bool | if "true" the json report will include a report on RTP transmission |
| srtp | string | Comma-separated values of the following "sdes" - add SDES support, "dtls" - add DTLS-SRTP support, "force" - make SRTP mandatory |
| hangup | int | call duration in second before hangup |
| label | string | test description or label |
| record | bool | if "true" the call will be recorded once connected in /voice_files |
| record_early | bool | if "true" the call will be recorded when early media starts in /voice_files. If call is answered after, recording will continue in the same file |

### call command parameters

| Name | Type | Description |
| ---- | ---- | ----------- |
| timer | string | control SIP session timers, possible values are : inactive, optional, required or always |
| proxy | string | ip/hostname of a proxy where to send the call |
| caller | string | From header user@host, only used if from it not specified |
| from | string | From header complete "\&quot;Display Name\&quot; <sip:test at 127.0.0.1>"  |
| callee | string | request URI user@host (also used in the To header unless to_uri is specified) |
| to_uri | string | used@host part of the URI in the To header |
| contact_uri_params | string | string, that will be added to Contact URI as params |
| transport | string | force a specific transport <tcp,udp,tls,sips> |
| re_invite_interval | int | Interval in seconds at which a re-invite with SDP will be sent |
| rtp_stats | bool | if "true" the json report will include a report on RTP transmission |
| srtp | string | Comma-separated values of the following "sdes" - add SDES support, "dtls" - add DTLS-SRTP support, "force" - make SRTP mandatory. Note, if you don't specify "force", call would be made with plain RTP. If you specify both "sdes" and "dtls", DTLS-SRTP would be used regardless of order. |
| late_start | bool | if "true" no SDP will be included in the INVITE and will result in a late offer in 200 OK/ACK |
| record | bool | if "true" the call will be recorded once connected in /voice_files |
| record_early | bool | if "true" the call will be recorded when early media starts in /voice_files. If call is answered after, recording will continue in the same file |
| force_contact | string | local contact header will be overwritten by the given string |
| max_ringing_duration | int | max ringing duration in seconds before cancel, default 60 |
| hangup | int | call duration in second before hangup |
| repeat | int | do this call multiple times |
| username | string | authentication username, account name, From/To/Contact header user part |
| password | string | authentication password |
| label | string | test description or label |

### register command parameters

| Name | Type | Description |
| ---- | ---- | ----------- |
| proxy | string | ip/hostname of a proxy where to send the register |
| username | string | authentication username, account name, From/To/Contact header user part |
| password | string | authentication password |
| account | string | if not specified username is used, this is the the account name and From/To/Contact header user part |
| registrar | string | SIP UAS handling registration where the messages will be sent |
| contact_uri_params | string | string, that will be added to Contact URI as params |
| transport | string | force a specific transport <tcp,udp,tls,sips> |
| unregister | bool | unregister the account <usename@registrar;transport=x> |
| reg_id | int | if present outbound and other related parameters will be added see RFC5626 |
| instance_id | int | same as reg_id, if not present, it will be generated automatically |
| rewrite_contact | bool | default true, detect public IP when registering and rewrite the contact header |
| srtp | string | Comma-separated values of the following "sdes" - add SDES support, "dtls" - add DTLS-SRTP support, "force" - make SRTP mandatory. Used for incoming calls to this account |
| account | string | if not specified username is used, this is the the account name and From/To/Contact header user part |
| registrar | string | SIP UAS handling registration where the messages will be sent |
| transport | string | force a specific transport <tcp,udp,tls,sips> |
| unregister | bool | unregister the account <usename@registrar;transport=x> |
| reg_id | int | if present outbound and other related parameters will be added see RFC5626 |
| instance_id | int | same as reg_id, if not present, it will be generated automatically |
| rewrite_contact | bool | default true, detect public IP when registering and rewrite the contact header |
| srtp | string | Comma-separated values of the following "sdes" - add SDES support, "dtls" - add DTLS-SRTP support, "force" - make SRTP mandatory. Used for incoming calls to this account |

### message command parameters

| Name | Type | Description |
| ---- | ---- | ----------- |
| from | string | From header complete "\&quot;Display Name\&quot; <sip:test at 127.0.0.1>"  |
| to_uri | string | used@host part of the URI in the To header |
| transport | string | force a specific transport <tcp,udp,tls,sips> |
| username | string | authentication username, account name, From/To/Contact header user part |
| password | string | authentication password |
| label | string | test description or label |

### Example: sending a message
```xml
<?xml version="1.0"?>
<config>
  <actions>
    <action type="message" label="testing SIP message" transport="udp"
      expected_cause_code="202"
      text="Message in a bottle."
      from="123456@in.the.ocean"
      to_uri="15876580542@in.the.ocean"
      username="123456"
      password="pass"
     />
    <action type="wait" complete="true"/>
  </actions>
</config>
```

### accept_message command parameters

| Name | Type | Description |
| ---- | ---- | ----------- |
| account | string | Account will be used if it matches the user part of an incoming message RURI or "default" will catch all |
| message_count | int | The amount of messages to receive to consider the command completed, default -1 (considered completed) |
| transport | string | Force a specific transport for all messages on accepted messages, default to all transport available |
| label | string | test description or label |

### Example: receiving a message
```xml
<?xml version="1.0"?>
<config>
  <actions>
    <action type="register" label="register" transport="udp"
      expected_cause_code="200"
      username="123456"
      password="password"
      registrar="pbx.somewhere.time"
     />
    <action type="wait" complete="true"/>
    <action type="accept_message" 
      account="123456"
      message_count="1"
     />
    <action type="wait" complete="true"/>
  </actions>
</config>
```

### wait command parameters

| Name | Type | Description |
| ---- | ---- | ----------- |
| complete | bool | if "true" wait for all the test to complete (or reach their wait_until state) before executing next action or disconnecting calls and exiting, needed in most cases |
| ms | int | the amount of milliseconds to wait before executing next action or disconnecting calls and exiting, if -1 wait forever |

### Example: codec configuration
```xml
<config>
  <actions>
    <action type="codec" disable="all"/>
    <action type="codec" enable="pcmu" priority="250"/>
    <!-- more actions ... -->
    <action type="wait" complete="true"/>
  </actions>
</config>
```

### codec command parameters

| Name | Type | Description |
| ---- | ---- | ----------- |
| priority | int | 0-255, where zero means to disable the codec |
| enable | string | Codec payload type ID, ex. "g722", "pcma", "opus" or "all" |
| disable | string | Codec payload type ID, ex. "g722", "pcma", "opus" or "all" |

### Example: TURN configuration
```xml
<config>
  <actions>
    <action type="turn" enabled="true" server="x.x.x.x:3478" username="foo" password="bar"/>
    <!-- more actions ... -->
    <action type="wait" complete="true"/>
  </actions>
</config>
```

### turn command parameters

| Name | Type | Description |
| ---- | ---- | ----------- |
| enabled | bool | if "true" turn server usage will be enabled |
| server | string | turn server URI or IP:port |
| username | string | turn server username |
| password | string | turn server password |
| password_hashed | bool | if "true" us hashed password, default plain password |
| sip_stun_use | bool | if "true" SIP reflective IP is use with signaling |
| media_stun_use | bool | if "true" STUN reflective IP is use with media/SDP |
| stun_only | bool | if "true" TURN and ICE are disabled and only STUN is use |

### using multiple accounts
When using multiple accounts, accounts can be created and selected with the following parameters.

|command | account parameter |
| ------ | ----------------- |
| accept | account |
| register | account |
| call | caller |
|accept_message| account |
| message | from |

### using env variable in scenario actions parameters
Any value starting with `VP_ENV` will be replaced by the envrironment variable of the same name.
Example : `username="VP_ENV_USERNAME"`
```bash
export VP_ENV_PASSWORD=????????
export VP_ENV_USERNAME=username
```

### Docker
```bash
voip_patrol/docker$ tree
.
├── build.sh        # docker build command example
├── Dockerfile      # docker build file for Linux Alpine
└── voip_patrol.sh  # docker run example starting
```

## Dependencies

#### PJSUA2
PJSUA2 : A C++ High Level Softphone API : built on top of PJSIP and PJMEDIA
http://www.pjsip.org
http://www.pjsip.org/docs/book-latest/PJSUA2Doc.pdf

## External tool to test audio quality

#### PESQ
P.862 : Perceptual evaluation of speech quality (PESQ): An objective method for end-to-end speech quality assessment of narrow-band telephone networks and speech codecs
http://www.itu.int/rec/T-REC-P.862
```
./run_pesq +16000 voice_files/reference.wav voice_files/recording.wav
```

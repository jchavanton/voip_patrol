## Example of load tests with voip_patrol

### this will start a multi window tmux with four instances of voip_patrol
[run.sh](run.sh)
```
./run.sh  "10.0.0.1:5060"
```

### validating results
Voip_patrol json test results can be use to confirm that the transmission quality was not degraded.
```
grep "action\": \"call\"" --no-filename  perf[1-5].json | jq . | grep mos_lq | sort | uniq -c
```

### notes
In the provided example scenario, each voip_patrol instance will do 100 calls and may expect receiving them back since it is registering the same callee.

```xml
<action type="call" label="outbound"                          
    callee="VP_ENV_CALLEE"                                    
    caller="perf_caller"                                      
    from="&quot;voip_patrol&quot; <sip:perf_test_client@lab>" 
    expected_cause_code="200"                                 
    rtp_stats="true"                                          
    hangup="25"                                               
    max_ringing_duration="120"                                
    play="../voice_ref_files/reference_8000.wav"              
    repeat=99"                                                
/>                                                             
```

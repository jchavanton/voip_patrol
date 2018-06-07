/*
 * Voip Patrol
 * @author Julien Chavanton 2018
 */


#include "voip_patrol.hh"

Action::Action(Config *cfg) : config{cfg} {
	std::cout<<"Prepared for Action!\n";
}

void Action::do_wait(bool complete_all, int duration_ms) {
	// config->wait(done);
	LOG(logINFO) << __FUNCTION__ << " duration_ms:" << duration_ms ;
	bool completed = false;
	int tests_running = 0;
	bool status_update = true;
	while (!completed) {
		for (auto & account : config->accounts) {
			AccountInfo acc_inf = account->getInfo();
			if (account->test && account->test->state == VPT_DONE){
				delete account->test;
				account->test = NULL;
			} else if (account->test) {
				tests_running++;
			}
		}
		for (auto & call : config->calls) {
			if (call->test && call->test->state == VPT_DONE){
				//LOG(logINFO) << "delete call test["<<call->test<<"]";
				//delete call->test;
				//call->test = NULL;
				//removeCall(call);
			} else if (call->test) {
				CallInfo ci = call->getInfo();
				if (status_update) {
					LOG(logINFO) <<"[wait:call]["<<call->getId()<<"][test]["<<(ci.role==0?"CALLER":"CALLEE")<<"]["
				  		     << ci.callIdString <<"]["<<ci.remoteUri<<"]["<<ci.stateText<<"|"<<ci.state<<"]duration["
						     << ci.connectDuration.sec <<">="<<call->test->hangup_duration<<"]";
				}
				if (ci.state == PJSIP_INV_STATE_CALLING || ci.state == PJSIP_INV_STATE_EARLY)  {
					if (call->test->max_calling_duration && call->test->max_calling_duration <= ci.totalDuration.sec) {
						LOG(logINFO) <<"[cancelling:call]["<<call->getId()<<"][test]["<<(ci.role==0?"CALLER":"CALLEE")<<"]["
				  		     << ci.callIdString <<"]["<<ci.remoteUri<<"]["<<ci.stateText<<"|"<<ci.state<<"]duration["
						     << ci.totalDuration.sec <<">="<<call->test->max_calling_duration<<"]";
						CallOpParam prm(true);
						call->hangup(prm);
					}
				} else if (ci.state == PJSIP_INV_STATE_CONFIRMED) {
					std::string res = "call[" + std::to_string(ci.lastStatusCode) + "] reason["+ ci.lastReason +"]";
					call->test->connect_duration = ci.connectDuration.sec;
					call->test->setup_duration = ci.totalDuration.sec - ci.connectDuration.sec;
					call->test->result_cause_code = (int)ci.lastStatusCode;
					call->test->reason = ci.lastReason;
					if (call->test->hangup_duration && ci.connectDuration.sec >= call->test->hangup_duration){
						if (ci.state == PJSIP_INV_STATE_CONFIRMED) {
							CallOpParam prm(true);
							LOG(logINFO) << "hangup : call in PJSIP_INV_STATE_CONFIRMED" ;
							call->hangup(prm);
						}
						if (call->role == 0 && call->test->min_mos > 0) {
							call->test->get_mos();
						}
						call->test->update_result();
					}
				}
				if (complete_all || call->test->state == VPT_RUN_WAIT)
					tests_running++;
			}
		}
		if (tests_running > 0) {
			if (status_update) {
				LOG(logINFO) <<LOG_COLOR_ERROR<<">>>> action[wait] active tests in run_wait["<<tests_running<<"] <<<<"<<LOG_COLOR_END;
				status_update = false;
			}
			tests_running=0;
			if (duration_ms > 0) duration_ms -= 100;
			pj_thread_sleep(100);
		} else {
			if (duration_ms > 0) {
				duration_ms -= 10;
				pj_thread_sleep(10);
				continue;
			}
			completed = true;
			LOG(logINFO) << "action[wait] completed";
			config->update_result(std::string("fds")+"action[wait] completed");
		}
	}
}

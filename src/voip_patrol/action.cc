/*
 * Voip Patrol
 * @author Julien Chavanton 2018
 */


#include "voip_patrol.hh"
#include "action.hh"

Action::Action(Config *cfg) : config{cfg} {
	init_actions_params();
	std::cout<<"Prepared for Action!\n";
}

vector<ActionParam>* Action::get_params(string name) {
	if (name.compare("call") == 0) return &do_call_params;
	if (name.compare("register") == 0) return &do_register_params;
	if (name.compare("wait") == 0) return &do_wait_params;
	if (name.compare("accept") == 0) return &do_accept_params;
	return nullptr;
}

string Action::get_env(string env) {
	if (const char* val = std::getenv(env.c_str())) {
		std::string s(val);
		return s;
	} else {
		return "";
	}
}

bool Action::set_param(ActionParam &param, const char *val) {
			if (!val) return false;
			if (param.type == APType::apt_integer) {
				param.i_val = atoi(val);
			} else {
				param.s_val = val;
				if (param.s_val.compare(0, 7, "VP_ENV_") == 0)
						param.s_val = get_env(val);
			}
			return true;
}

void Action::init_actions_params() {
	// do_call
	do_call_params.push_back(ActionParam("caller", true, APType::apt_string));
	do_call_params.push_back(ActionParam("callee", true, APType::apt_string));
	do_call_params.push_back(ActionParam("label", false, APType::apt_string));
	do_call_params.push_back(ActionParam("username", false, APType::apt_string));
	do_call_params.push_back(ActionParam("password", false, APType::apt_string));
	do_call_params.push_back(ActionParam("realm", false, APType::apt_string));
	do_call_params.push_back(ActionParam("transport", false, APType::apt_string));
	do_call_params.push_back(ActionParam("expected_cause_code", false, APType::apt_integer));
	do_call_params.push_back(ActionParam("wait_until", false, APType::apt_integer));
	do_call_params.push_back(ActionParam("max_duration", false, APType::apt_integer));
	do_call_params.push_back(ActionParam("hangup", false, APType::apt_integer));
	// do_register
	do_register_params.push_back(ActionParam("transport", false, APType::apt_string));
	do_register_params.push_back(ActionParam("label", false, APType::apt_string));
	do_register_params.push_back(ActionParam("registrar", false, APType::apt_string));
	do_register_params.push_back(ActionParam("realm", false, APType::apt_string));
	do_register_params.push_back(ActionParam("username", false, APType::apt_string));
	do_register_params.push_back(ActionParam("password", false, APType::apt_string));
	do_register_params.push_back(ActionParam("expected_cause_code", false, APType::apt_integer));
	// do_accept
	do_accept_params.push_back(ActionParam("account", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("transport", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("label", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("max_duration", false, APType::apt_integer));
	do_accept_params.push_back(ActionParam("hangup", false, APType::apt_integer));
	// do_wait
	do_wait_params.push_back(ActionParam("ms", false, APType::apt_integer));
	do_wait_params.push_back(ActionParam("complete", false, APType::apt_integer));
}

void Action::do_register(vector<ActionParam> &params) {
	string type("register");
	string transport {};
	string label {};
	string registrar {};
	string realm {};
	string username {};
	string password {};
	int expected_cause_code {200};

	for (auto param : params) {
		if (param.name.compare("transport") == 0) transport = param.s_val;
		else if (param.name.compare("label") == 0) label = param.s_val;
		else if (param.name.compare("registrar") == 0) registrar = param.s_val;
		else if (param.name.compare("realm") == 0) realm = param.s_val;
		else if (param.name.compare("username") == 0) username = param.s_val;
		else if (param.name.compare("password") == 0) password = param.s_val;
		else if (param.name.compare("expected_cause_code") == 0) expected_cause_code = param.i_val;
	}

	if (username.empty() || realm.empty() || password.empty() || registrar.empty()) {
		LOG(logERROR) <<__FUNCTION__<<" missing action parameter" ;
		return;
	}

	Test *test = new Test(config, type);
	test->local_user = username;
	test->remote_user = username;
	test->label = label;
	test->expected_cause_code = expected_cause_code;
	test->from = username;
	test->type = type;

	LOG(logINFO) <<__FUNCTION__<< "sip:" + username + "@" + registrar  ;
	AccountConfig acc_cfg;
	SipHeader sh;
	sh.hName = "User-Agent";
	sh.hValue = "<voip_patrol>";
	acc_cfg.regConfig.headers.push_back(sh);

	acc_cfg.sipConfig.transportId = config->transport_id_udp;
	if (!transport.empty()) {
		if (transport.compare("tcp") == 0) {
			acc_cfg.sipConfig.transportId = config->transport_id_tcp;
		} else if (transport.compare("tls") == 0) {
			if (config->transport_id_tls == -1) {
				LOG(logERROR) <<__FUNCTION__<<" TLS transport not supported";
				return;
			}
			acc_cfg.sipConfig.transportId = config->transport_id_tls;
		}
	}
	if (acc_cfg.sipConfig.transportId == config->transport_id_tls) {
		acc_cfg.idUri = "sips:" + username + "@" + registrar;
		acc_cfg.regConfig.registrarUri = "sips:" + registrar;
		LOG(logINFO) <<__FUNCTION__<< " SIPS URI Scheme";
	} else {
		LOG(logINFO) <<__FUNCTION__<< " SIP URI Scheme";
		acc_cfg.idUri = "sip:" + username + "@" + registrar;
		acc_cfg.regConfig.registrarUri = "sip:" + registrar;
	}
	acc_cfg.sipConfig.authCreds.push_back( AuthCredInfo("digest", realm, username, 0, password) );

	TestAccount *acc = config->findAccount(username);
	if (!acc) {
		acc = config->createAccount(acc_cfg);
	} else {
		acc->modify(acc_cfg);
	}
	acc->setTest(test);
}

void Action::do_accept(vector<ActionParam> &params) {
	string type("accept");
	string account_name {};
	string transport {};
	string label {};
	int max_duration {0};
	int hangup_duration {0};

	for (auto param : params) {
		if (param.name.compare("account") == 0) account_name = param.s_val;
		else if (param.name.compare("transport") == 0) transport = param.s_val;
		else if (param.name.compare("label") == 0) label = param.s_val;
		else if (param.name.compare("max_duration") == 0) max_duration = param.i_val;
		else if (param.name.compare("hangup") == 0) hangup_duration = param.i_val;
	}

	if (account_name.empty()) {
		LOG(logERROR) <<__FUNCTION__<<" missing action parameters <account>" ;
		return;
	}

	TestAccount *acc = config->findAccount(account_name);
	if (!acc) {
		AccountConfig acc_cfg;
		acc_cfg.sipConfig.transportId = config->transport_id_udp;
		if (!transport.empty()) {
			if (transport.compare("tcp") == 0) {
				acc_cfg.sipConfig.transportId = config->transport_id_tcp;
			} else if (transport.compare("tls") == 0) {
				if (config->transport_id_tls == -1) {
					LOG(logERROR) <<__FUNCTION__<<": TLS transport not supported.";
					return;
				}
				acc_cfg.sipConfig.transportId = config->transport_id_tls;
			}
		}
		if (acc_cfg.sipConfig.transportId == config->transport_id_tls) {
			acc_cfg.idUri = "sips:" + account_name;
		} else {
			acc_cfg.idUri = "sip:" + account_name;
		}
		acc = config->createAccount(acc_cfg);
	}
	acc->hangup_duration = hangup_duration;
	acc->max_duration = max_duration;
	acc->accept_label = label;
}

void Action::do_call(vector<ActionParam> &params) {
	string type("call");
	string caller {};
	string callee {};
	string transport {};
	string username {};
	string password {};
	string realm {};
	string label {};
	int expected_cause_code {200};
	int wait_until {0};
	float min_mos {0.0};
	int max_duration {0};
	int max_calling_duration {0};
	int expected_duration {0};
	int hangup_duration {0};
	int repeat {0};
	bool recording {false};

	for (auto param : params) {
		if (param.name.compare("callee") == 0) callee = param.s_val;
		else if (param.name.compare("caller") == 0) caller = param.s_val;
		else if (param.name.compare("transport") == 0) transport = param.s_val;
		else if (param.name.compare("username") == 0) username = param.s_val;
		else if (param.name.compare("password") == 0) password = param.s_val;
		else if (param.name.compare("realm") == 0) realm = param.s_val;
		else if (param.name.compare("label") == 0) label = param.s_val;
		else if (param.name.compare("expected_cause_code") == 0) expected_cause_code = param.i_val;
		else if (param.name.compare("wait_until") == 0) wait_until = param.i_val;
		else if (param.name.compare("min_mos") == 0) min_mos = param.f_val;
		else if (param.name.compare("max_duration") == 0) max_duration = param.i_val;
		else if (param.name.compare("max_calling_duration") == 0) max_calling_duration = param.i_val;
		else if (param.name.compare("duration") == 0) expected_duration = param.i_val;
		else if (param.name.compare("hangup") == 0) hangup_duration = param.i_val;
		else if (param.name.compare("repeat") == 0) repeat = param.i_val;
		else if (param.name.compare("recording") == 0) recording = true;
	}

	if (caller.empty() || callee.empty()) {
		LOG(logERROR) <<__FUNCTION__<<": missing action parameters for callee/caller" ;
		return;
	}

	TestAccount* acc = config->findAccount(caller);
	if (!acc) {
		AccountConfig acc_cfg;
		acc_cfg.sipConfig.transportId = config->transport_id_udp;
		if (!transport.empty()) {
			if (transport.compare("tcp") == 0) {
				acc_cfg.sipConfig.transportId = config->transport_id_tcp;
			} else if (transport.compare("tls") == 0) {
				if (config->transport_id_tls == -1) {
					LOG(logERROR) <<__FUNCTION__<<": TLS transport not supported" ;
					return;
				}
				acc_cfg.sipConfig.transportId = config->transport_id_tls;
			}
		}
		if (acc_cfg.sipConfig.transportId == config->transport_id_tls) {
			acc_cfg.idUri = "sips:" + caller;
		} else {
			acc_cfg.idUri = "sip:" + caller;
		}
		if (!realm.empty()) {
			if (username.empty() || password.empty()) {
				if (username.empty()) LOG(logERROR) <<__FUNCTION__<< ": realm specified missing username";
				else LOG(logERROR) <<__FUNCTION__<<": realm specified missing password";
				return;
			}
			acc_cfg.sipConfig.authCreds.push_back( AuthCredInfo("digest", realm, username, 0, password) );
		}
		acc = config->createAccount(acc_cfg);
	}

	do {
		Test *test = new Test(config, type);
		test->wait_state = (call_wait_state_t)wait_until;
		test->min_mos = min_mos;
		test->expected_duration = expected_duration;
		test->label = label;
		test->max_duration = max_duration;
		test->max_calling_duration = max_calling_duration;
		test->hangup_duration = hangup_duration;
		test->recording = recording;

		std::size_t pos = caller.find("@");
		if (pos!=std::string::npos) {
			test->local_user = caller.substr(0, pos);
		}
		pos = callee.find("@");
		if (pos!=std::string::npos) {
			test->remote_user = callee.substr(0, pos);
		}

		TestCall *call = new TestCall(acc);
		config->calls.push_back(call);

		call->test = test;
		test->expected_cause_code = expected_cause_code;
		test->from = caller;
		test->to = callee;
		test->type = type;
		acc->calls.push_back(call);
		CallOpParam prm(true);
		prm.opt.audioCount = 1;
		prm.opt.videoCount = 0;
		LOG(logINFO) << "call->test:" << test << " " << call->test->type;
		LOG(logINFO) << "calling :" +callee;
		if (transport.compare("tls") == 0) {
			call->makeCall("sips:"+callee, prm);
		} else if (transport.compare("tcp") == 0) {
			call->makeCall("sip:"+callee+";transport=tcp", prm);
		} else {
			call->makeCall("sip:"+callee, prm);
		}
		repeat--;
	} while (repeat >= 0);
}

void Action::do_wait(vector<ActionParam> &params) {
	int duration_ms = 0;
	int complete_all = 0;
	for (auto param : params) {
		if (param.name.compare("ms") == 0) duration_ms = param.i_val;
		if (param.name.compare("complete") == 0) complete_all = param.i_val;
	}
	LOG(logINFO) << __FUNCTION__ << " duration_ms:" << duration_ms << " complete all tests:" << complete_all;
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
					LOG(logDEBUG) <<__FUNCTION__<<": [call]["<<call->getId()<<"][test]["<<(ci.role==0?"CALLER":"CALLEE")<<"]["
				  		     << ci.callIdString <<"]["<<ci.remoteUri<<"]["<<ci.stateText<<"|"<<ci.state<<"]duration["
						     << ci.connectDuration.sec <<">="<<call->test->hangup_duration<<"]";
				}
				if (ci.state == PJSIP_INV_STATE_CALLING || ci.state == PJSIP_INV_STATE_EARLY)  {
					if (call->test->max_calling_duration && call->test->max_calling_duration <= ci.totalDuration.sec) {
						LOG(logINFO) <<__FUNCTION__<<"[cancelling:call]["<<call->getId()<<"][test]["<<(ci.role==0?"CALLER":"CALLEE")<<"]["
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
				LOG(logINFO) <<__FUNCTION__<<LOG_COLOR_ERROR<<": action[wait] active tests in run_wait["<<tests_running<<"] <<<<"<<LOG_COLOR_END;
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
			} else if (duration_ms == -1) {
				pj_thread_sleep(10);
				continue;
			}
			completed = true;
			LOG(logINFO) <<__FUNCTION__<<": completed";
			config->update_result(std::string("fds")+"action[wait] completed");
		}
	}
}

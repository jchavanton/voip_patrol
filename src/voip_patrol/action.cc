/*
 * Copyright (C) 2016-2018 Julien Chavanton <jchavanton@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA~
 */

#include "voip_patrol.hh"
#include "action.hh"
#include "string.h"

Action::Action(Config *cfg) : config{cfg} {
	init_actions_params();
	std::cout<<"Prepared for Action!\n";
}

vector<ActionParam> Action::get_params(string name) {
	if (name.compare("call") == 0) return do_call_params;
	else if (name.compare("register") == 0) return do_register_params;
	else if (name.compare("wait") == 0) return do_wait_params;
	else if (name.compare("accept") == 0) return do_accept_params;
	else if (name.compare("alert") == 0) return do_alert_params;
	else if (name.compare("codec") == 0) return do_codec_params;
	vector<ActionParam> empty_params;
	return empty_params;
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
			LOG(logINFO) <<__FUNCTION__<< " param name:" << param.name << " val:" << val; 
			if (param.type == APType::apt_bool) {
				if( strcmp(val, "false") ==  0 )  param.b_val = false;
				else param.b_val = true;
			} else if (param.type == APType::apt_integer) {
				param.i_val = atoi(val);
			} else if (param.type == APType::apt_float) {
				param.f_val = atof(val);
			} else {
				param.s_val = val;
				if (param.s_val.compare(0, 7, "VP_ENV_") == 0)
						param.s_val = get_env(val);
			}
			return true;
}

bool Action::set_param_by_name(vector<ActionParam> *params, const string name, const char *val) {
	for (auto &param : *params) {
		if (param.name.compare(name) == 0) {
				return set_param(param, val);
		}
	}
	return false;
}

void Action::init_actions_params() {
	// do_call
	do_call_params.push_back(ActionParam("caller", true, APType::apt_string));
	do_call_params.push_back(ActionParam("from", true, APType::apt_string));
	do_call_params.push_back(ActionParam("callee", true, APType::apt_string));
	do_call_params.push_back(ActionParam("to_uri", true, APType::apt_string));
	do_call_params.push_back(ActionParam("label", false, APType::apt_string));
	do_call_params.push_back(ActionParam("username", false, APType::apt_string));
	do_call_params.push_back(ActionParam("password", false, APType::apt_string));
	do_call_params.push_back(ActionParam("realm", false, APType::apt_string));
	do_call_params.push_back(ActionParam("transport", false, APType::apt_string));
	do_call_params.push_back(ActionParam("expected_cause_code", false, APType::apt_integer));
	do_call_params.push_back(ActionParam("wait_until", false, APType::apt_string));
	do_call_params.push_back(ActionParam("max_duration", false, APType::apt_integer));
	do_call_params.push_back(ActionParam("min_mos", false, APType::apt_float));
	do_call_params.push_back(ActionParam("rtp_stats", false, APType::apt_bool));
	do_call_params.push_back(ActionParam("late_start", false, APType::apt_bool));
	do_call_params.push_back(ActionParam("force_contact", false, APType::apt_string));
	do_call_params.push_back(ActionParam("hangup", false, APType::apt_integer));
	do_call_params.push_back(ActionParam("re_invite_interval", false, APType::apt_integer));
	do_call_params.push_back(ActionParam("play", false, APType::apt_string));
	do_call_params.push_back(ActionParam("play_dtmf", false, APType::apt_string));
	do_call_params.push_back(ActionParam("timer", false, APType::apt_string));
	do_call_params.push_back(ActionParam("proxy", false, APType::apt_string));
	// do_register
	do_register_params.push_back(ActionParam("transport", false, APType::apt_string));
	do_register_params.push_back(ActionParam("label", false, APType::apt_string));
	do_register_params.push_back(ActionParam("registrar", false, APType::apt_string));
	do_register_params.push_back(ActionParam("proxy", false, APType::apt_string));
	do_register_params.push_back(ActionParam("realm", false, APType::apt_string));
	do_register_params.push_back(ActionParam("username", false, APType::apt_string));
	do_register_params.push_back(ActionParam("account", false, APType::apt_string));
	do_register_params.push_back(ActionParam("password", false, APType::apt_string));
	do_register_params.push_back(ActionParam("unregister", false, APType::apt_bool));
	do_register_params.push_back(ActionParam("expected_cause_code", false, APType::apt_integer));
	// do_accept
	do_accept_params.push_back(ActionParam("account", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("transport", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("label", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("max_duration", false, APType::apt_integer));
	do_accept_params.push_back(ActionParam("ring_duration", false, APType::apt_integer));
	do_accept_params.push_back(ActionParam("response_delay", false, APType::apt_integer));
	do_accept_params.push_back(ActionParam("early_media", false, APType::apt_bool));
	do_accept_params.push_back(ActionParam("wait_until", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("hangup", false, APType::apt_integer));
	do_accept_params.push_back(ActionParam("re_invite_interval", false, APType::apt_integer));
	do_accept_params.push_back(ActionParam("min_mos", false, APType::apt_float));
	do_accept_params.push_back(ActionParam("rtp_stats", false, APType::apt_bool));
	do_accept_params.push_back(ActionParam("late_start", false, APType::apt_bool));
	do_accept_params.push_back(ActionParam("play", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("code", false, APType::apt_integer));
	do_accept_params.push_back(ActionParam("call_count", false, APType::apt_integer));
	do_accept_params.push_back(ActionParam("reason", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("play_dtmf", false, APType::apt_string));
	do_accept_params.push_back(ActionParam("timer", false, APType::apt_string));
	// do_wait
	do_wait_params.push_back(ActionParam("ms", false, APType::apt_integer));
	do_wait_params.push_back(ActionParam("complete", false, APType::apt_bool));
	// do_alert
	do_alert_params.push_back(ActionParam("email", false, APType::apt_string));
	do_alert_params.push_back(ActionParam("email_from", false, APType::apt_string));
	do_alert_params.push_back(ActionParam("smtp_host", false, APType::apt_string));
	// do_codec
	do_codec_params.push_back(ActionParam("priority", false, APType::apt_integer));
	do_codec_params.push_back(ActionParam("enable", false, APType::apt_string));
	do_codec_params.push_back(ActionParam("disable", false, APType::apt_string));
}

void Action::do_register(vector<ActionParam> &params, vector<ActionCheck> &checks, SipHeaderVector &x_headers) {
	string type {"register"};
	string transport {};
	string label {};
	string registrar {};
	string proxy {};
	string realm {};
	string username {};
	string account_name {};
	string password {};
	int expected_cause_code {200};
	bool unregister {false};

	for (auto param : params) {
		if (param.name.compare("transport") == 0) transport = param.s_val;
		else if (param.name.compare("label") == 0) label = param.s_val;
		else if (param.name.compare("registrar") == 0) registrar = param.s_val;
		else if (param.name.compare("proxy") == 0) proxy = param.s_val;
		else if (param.name.compare("realm") == 0) realm = param.s_val;
		else if (param.name.compare("account") == 0) account_name = param.s_val;
		else if (param.name.compare("username") == 0) username = param.s_val;
		else if (param.name.compare("password") == 0) password = param.s_val;
		else if (param.name.compare("unregister") == 0) unregister = param.b_val;
		else if (param.name.compare("expected_cause_code") == 0) expected_cause_code = param.i_val;
	}

	if (username.empty() || realm.empty() || password.empty() || registrar.empty()) {
		LOG(logERROR) <<__FUNCTION__<<" missing action parameter" ;
		return;
	}

	if (account_name.empty()) account_name = username;
	account_name = account_name + "@" + registrar;
	TestAccount *acc = config->findAccount(account_name);
	if (unregister) {
		if (acc) {
			// We should probably create a new test ...
			if (acc->test) acc->test->type = "unregister";
			LOG(logINFO) <<__FUNCTION__<< " unregister ("<<account_name<<")";
			AccountInfo acc_inf = acc->getInfo();
			if (acc_inf.regIsActive) {
				LOG(logINFO) <<__FUNCTION__<< " register is active";
				try {
					acc->setRegistration(false);
				} catch (pj::Error e)  {
					LOG(logERROR) <<__FUNCTION__<<" error :" << e.status << std::endl;
				}
			} else {
				LOG(logINFO) <<__FUNCTION__<< " register is not active";
			}
			while (acc_inf.regIsActive) {
				pj_thread_sleep(10);
				acc_inf = acc->getInfo();
			}
			return;
		} else {
			LOG(logINFO) <<__FUNCTION__<< "unregister: account not found ("<<account_name<<")";
		}
	}

	Test *test = new Test(config, type);
	test->local_user = username;
	test->remote_user = username;
	test->label = label;
	test->expected_cause_code = expected_cause_code;
	test->from = username;
	test->type = type;

	LOG(logINFO) <<__FUNCTION__<< " >> sip:" + account_name;
	AccountConfig acc_cfg;
	SipHeader sh;
	sh.hName = "User-Agent";
	sh.hValue = "<voip_patrol>";
	acc_cfg.regConfig.headers.push_back(sh);
	for (auto x_hdr : x_headers) {
		acc_cfg.regConfig.headers.push_back(x_hdr);
	}

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
		acc_cfg.idUri = "sips:" + account_name;
		acc_cfg.regConfig.registrarUri = "sips:" + registrar;
		if (!proxy.empty())
			acc_cfg.sipConfig.proxies.push_back("sips:" + proxy);

		LOG(logINFO) <<__FUNCTION__<< " SIPS URI Scheme";
	} else {
		LOG(logINFO) <<__FUNCTION__<< " SIP URI Scheme";
		acc_cfg.idUri = "sip:" + account_name;
		acc_cfg.regConfig.registrarUri = "sip:" + registrar;
		if (!proxy.empty())
			acc_cfg.sipConfig.proxies.push_back("sip:" + proxy);

	}
	acc_cfg.sipConfig.authCreds.push_back( AuthCredInfo("digest", realm, username, 0, password) );

	if (!acc) {
		acc = config->createAccount(acc_cfg);
	} else {
		acc->modify(acc_cfg);
	}
	acc->setTest(test);
}

void Action::do_accept(vector<ActionParam> &params, vector<ActionCheck> &checks, pj::SipHeaderVector &x_headers) {
	string type {"accept"};
	string account_name {};
	string transport {};
	string label {};
	string play {default_playback_file};
	string play_dtmf {};
	string timer {};
	float min_mos {0.0};
	int max_duration {0};
	int ring_duration {0};
	int early_media {false};
	int hangup_duration {0};
	int re_invite_interval {0};
	call_state_t wait_until {INV_STATE_NULL};
	bool rtp_stats {false};
	bool late_start {false};
	int code {200};
	int call_count {-1};
	int response_delay {0};
	string reason {};

	for (auto param : params) {
		if (param.name.compare("account") == 0) account_name = param.s_val;
		else if (param.name.compare("transport") == 0) transport = param.s_val;
		else if (param.name.compare("play") == 0 && param.s_val.length() > 0) play = param.s_val;
		else if (param.name.compare("play_dtmf") == 0 && param.s_val.length() > 0) play_dtmf = param.s_val;
		else if (param.name.compare("timer") == 0 && param.s_val.length() > 0) timer = param.s_val;
		else if (param.name.compare("code") == 0) code = param.i_val;
		else if (param.name.compare("call_count") == 0) call_count = param.i_val;
		else if (param.name.compare("reason") == 0 && param.s_val.length() > 0) reason = param.s_val;
		else if (param.name.compare("label") == 0) label = param.s_val;
		else if (param.name.compare("max_duration") == 0) max_duration = param.i_val;
		else if (param.name.compare("ring_duration") == 0) ring_duration = param.i_val;
		else if (param.name.compare("early_media") == 0) early_media = param.b_val;
		else if (param.name.compare("min_mos") == 0) min_mos = param.f_val;
		else if (param.name.compare("rtp_stats") == 0) rtp_stats = param.b_val;
		else if (param.name.compare("late_start") == 0) late_start = param.b_val;
		else if (param.name.compare("wait_until") == 0) wait_until = get_call_state_from_string(param.s_val);
		else if (param.name.compare("hangup") == 0) hangup_duration = param.i_val;
		else if (param.name.compare("re_invite_interval") == 0) re_invite_interval = param.i_val;
		else if (param.name.compare("response_delay") == 0) response_delay = param.i_val;
	}

	if (account_name.empty()) {
		LOG(logERROR) <<__FUNCTION__<<" missing action parameters <account>" ;
		return;
	}

	TestAccount *acc = config->findAccount(account_name);
	if (!acc) {
		AccountConfig acc_cfg;
		if (!transport.empty()) {
			if (transport.compare("tcp") == 0) {
				acc_cfg.sipConfig.transportId = config->transport_id_tcp;
			} else if (transport.compare("udp") == 0) {
				acc_cfg.sipConfig.transportId = config->transport_id_udp;
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
		if (!timer.empty()) {
			if (timer.compare("inactive") == 0) {
				acc_cfg.callConfig.timerUse = PJSUA_SIP_TIMER_INACTIVE;
			} else if (timer.compare("optionnal") == 0) {
				acc_cfg.callConfig.timerUse = PJSUA_SIP_TIMER_OPTIONAL;
			} else if (timer.compare("required") == 0) {
				acc_cfg.callConfig.timerUse = PJSUA_SIP_TIMER_REQUIRED;
			} else if (timer.compare("always") == 0) {
				acc_cfg.callConfig.timerUse = PJSUA_SIP_TIMER_ALWAYS;
			}
			LOG(logINFO) <<__FUNCTION__<<":session timer["<<timer<<"]: "<< acc_cfg.callConfig.timerUse ;
		}
		acc = config->createAccount(acc_cfg);
	}
	acc->hangup_duration = hangup_duration;
	acc->re_invite_interval = re_invite_interval;
	acc->response_delay = response_delay;
	acc->max_duration = max_duration;
	acc->ring_duration = ring_duration;
	acc->accept_label = label;
	acc->rtp_stats = rtp_stats;
	acc->late_start= late_start;
	acc->play = play;
	acc->play_dtmf = play_dtmf;
	acc->timer = timer;
	acc->early_media = early_media;
	acc->wait_state = wait_until;
	acc->reason = reason;
	acc->code = code;
	acc->call_count = call_count;
	acc->x_headers = x_headers;
	acc->checks = checks;
}


void Action::do_call(vector<ActionParam> &params, vector<ActionCheck> &checks, SipHeaderVector &x_headers) {
	string type {"call"};
	string play {default_playback_file};
	string play_dtmf {};
	string timer {};
	string caller {};
	string from {};
	string callee {};
	string to_uri {};
	string transport {};
	string username {};
	string password {};
	string realm {};
	string label {};
	string proxy {};
	int expected_cause_code {200};
	call_state_t wait_until {INV_STATE_NULL};
	float min_mos {0.0};
	int max_duration {0};
	int max_calling_duration {0};
	int expected_duration {0};
	int hangup_duration {0};
	int re_invite_interval {0};
	int repeat {0};
	bool recording {false};
	bool rtp_stats {false};
	bool late_start {false};
	string force_contact {};

	for (auto param : params) {
		if (param.name.compare("callee") == 0) callee = param.s_val;
		else if (param.name.compare("caller") == 0) caller = param.s_val;
		else if (param.name.compare("from") == 0) from = param.s_val;
		else if (param.name.compare("to_uri") == 0) to_uri = param.s_val;
		else if (param.name.compare("transport") == 0) transport = param.s_val;
		else if (param.name.compare("play") == 0 && param.s_val.length() > 0) play = param.s_val;
		else if (param.name.compare("play_dtmf") == 0 && param.s_val.length() > 0) play_dtmf = param.s_val;
		else if (param.name.compare("timer") == 0 && param.s_val.length() > 0) timer = param.s_val;
		else if (param.name.compare("username") == 0) username = param.s_val;
		else if (param.name.compare("password") == 0) password = param.s_val;
		else if (param.name.compare("realm") == 0) realm = param.s_val;
		else if (param.name.compare("label") == 0) label = param.s_val;
		else if (param.name.compare("proxy") == 0) proxy = param.s_val;
		else if (param.name.compare("expected_cause_code") == 0) expected_cause_code = param.i_val;
		else if (param.name.compare("wait_until") == 0) wait_until = get_call_state_from_string(param.s_val);
		else if (param.name.compare("min_mos") == 0) min_mos = param.f_val;
		else if (param.name.compare("rtp_stats") == 0) rtp_stats = param.b_val;
		else if (param.name.compare("late_start") == 0) late_start = param.b_val;
		else if (param.name.compare("force_contact") == 0) force_contact = param.s_val;
		else if (param.name.compare("max_duration") == 0) max_duration = param.i_val;
		else if (param.name.compare("max_calling_duration") == 0) max_calling_duration = param.i_val;
		else if (param.name.compare("duration") == 0) expected_duration = param.i_val;
		else if (param.name.compare("hangup") == 0) hangup_duration = param.i_val;
		else if (param.name.compare("re_invite_interval") == 0) re_invite_interval = param.i_val;
		else if (param.name.compare("repeat") == 0) repeat = param.i_val;
		else if (param.name.compare("recording") == 0) recording = true;
	}

	if (caller.empty() || callee.empty()) {
		LOG(logERROR) <<__FUNCTION__<<": missing action parameters for callee/caller" ;
		return;
	}

	string account_uri {caller};
	if (transport.compare("udp") != 0)
		account_uri = caller + ";transport=" + transport;
	TestAccount* acc = config->findAccount(account_uri);
	if (!acc) {
		AccountConfig acc_cfg;
		if ( force_contact.compare("") != 0){
			LOG(logINFO) <<__FUNCTION__<< ":do_call:force_contact:"<< force_contact << "\n"; 
			acc_cfg.sipConfig.contactForced = force_contact;
		}
		if (!proxy.empty())
			acc_cfg.sipConfig.proxies.push_back("sip:" + proxy);
		if (!timer.empty()) {
			if (timer.compare("inactive") == 0) {
				acc_cfg.callConfig.timerUse = PJSUA_SIP_TIMER_INACTIVE;
			} else if (timer.compare("optionnal") == 0) {
				acc_cfg.callConfig.timerUse = PJSUA_SIP_TIMER_OPTIONAL;
			} else if (timer.compare("required") == 0) {
				acc_cfg.callConfig.timerUse = PJSUA_SIP_TIMER_REQUIRED;
			} else if (timer.compare("always") == 0) {
				acc_cfg.callConfig.timerUse = PJSUA_SIP_TIMER_ALWAYS;
			}
			LOG(logERROR) <<__FUNCTION__<<": session timer["<<timer<<"] : "<< acc_cfg.callConfig.timerUse ;
		}
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
			acc_cfg.idUri = "sips:" + account_uri;
		} else {
			acc_cfg.idUri = "sip:" + account_uri;
		}
		if (!from.empty())
			acc_cfg.idUri = from;
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
		test->wait_state = wait_until;
		if (test->wait_state != INV_STATE_NULL)
			test->state = VPT_RUN_WAIT;
		test->expected_duration = expected_duration;
		test->label = label;
		test->play = play;
		test->play_dtmf = play_dtmf;
		test->min_mos = min_mos;
		test->max_duration = max_duration;
		test->max_calling_duration = max_calling_duration;
		test->hangup_duration = hangup_duration;
		test->re_invite_interval = re_invite_interval;
		test->re_invite_next = re_invite_interval;
		test->recording = recording;
		test->rtp_stats = rtp_stats;
		test->late_start = late_start;
		test->force_contact = force_contact;
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

		for (auto x_hdr : x_headers) {
			prm.txOption.headers.push_back(x_hdr);
		}

		prm.opt.audioCount = 1;
		prm.opt.videoCount = 0;
		LOG(logINFO) << "call->test:" << test << " " << call->test->type;
		LOG(logINFO) << "calling :" +callee;
		if (transport.compare("tls") == 0) {
			if (!to_uri.empty())
					to_uri = "sips:"+to_uri;
			try {
				call->makeCall("sips:"+callee, prm, to_uri);
			} catch (pj::Error e)  {
				LOG(logERROR) <<__FUNCTION__<<" error :" << e.status << std::endl;
			}
		} else if (transport.compare("tcp") == 0) {
			if (!to_uri.empty())
					to_uri = "sip:"+to_uri+";transport=tcp";
			try {
				call->makeCall("sip:"+callee+";transport=tcp", prm, to_uri);
			} catch (pj::Error e)  {
				LOG(logERROR) <<__FUNCTION__<<" error :" << e.status << std::endl;
			}
		} else {
			if (!to_uri.empty())
					to_uri = "sip:"+to_uri;
			try {
				call->makeCall("sip:"+callee, prm, to_uri);
			} catch (pj::Error e)  {
				LOG(logERROR) <<__FUNCTION__<<" error :" << e.status << std::endl;
			}
		}
		repeat--;
	} while (repeat >= 0);
}

void Action::do_codec(vector<ActionParam> &params) {
	string enable {};
	int priority {0};
	string disable {};
	for (auto param : params) {
		if (param.name.compare("enable") == 0) enable = param.s_val;
		else if (param.name.compare("priority") == 0) priority = param.i_val;
		else if (param.name.compare("disable") == 0) disable = param.s_val;
	}
	LOG(logINFO) << __FUNCTION__ << " enable["<<enable<<"] with priority["<<priority<<"] disable["<<disable<<"]";
	if (!config->ep) {
		LOG(logERROR) << __FUNCTION__ << " PJSIP endpoint not available";
		return;
	}
	if (!disable.empty())
		config->ep->setCodecs(disable, 0);
	if (!enable.empty())
		config->ep->setCodecs(enable, priority);
}

void Action::do_alert(vector<ActionParam> &params) {
	string email {};
	string email_from {};
	string smtp_host {};
	for (auto param : params) {
		if (param.name.compare("email") == 0) email = param.s_val;
		else if (param.name.compare("email_from") == 0) email_from = param.s_val;
		else if (param.name.compare("smtp_host") == 0) smtp_host = param.s_val;
	}
	LOG(logINFO) << __FUNCTION__ << "email to:"<<email<< " from:"<<email_from;
	config->alert_email_to = email;
	config->alert_email_from = email_from;
	config->alert_server_url = smtp_host;
}

void Action::do_wait(vector<ActionParam> &params) {
	int duration_ms = 0;
	bool complete_all = false;
	for (auto param : params) {
		if (param.name.compare("ms") == 0) duration_ms = param.i_val;
		if (param.name.compare("complete") == 0) complete_all = param.b_val;
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
			if (account->call_count > 0 && duration_ms > 0) {
				tests_running++;
			}
		}
		for (auto & call : config->calls) {
			if (call->test && call->test->state == VPT_DONE){
				//CallInfo ci = call->getInfo();
				//if (ci.state == PJSIP_INV_STATE_DISCONNECTED)
				//LOG(logINFO) << "delete call test["<<call->test<<"] = " << config->removeCall(call);
				continue;
			} else if (call->test) {
				CallInfo ci = call->getInfo();
				if (status_update) {
					LOG(logDEBUG) <<__FUNCTION__<<": [call]["<<call->getId()<<"][test]["<<(ci.role==0?"CALLER":"CALLEE")<<"]["
						     << ci.callIdString <<"]["<<ci.remoteUri<<"]["<<ci.stateText<<"|"<<ci.state<<"]duration["
						     << ci.connectDuration.sec <<">="<<call->test->hangup_duration<<"]";
				}
				if (ci.state == PJSIP_INV_STATE_CALLING || ci.state == PJSIP_INV_STATE_EARLY)  {
					Test *test = call->test;
					if (test->ring_duration > 0 && ci.totalDuration.sec >= test->ring_duration) {
						CallOpParam prm;
						//if (test->reason.size() > 0) prm.reason = test->reason;
						prm.reason = "OK";
						if (test->code) prm.statusCode = test->code;
						else prm.statusCode = PJSIP_SC_OK;
						call->answer(prm);
					} else if (test->max_calling_duration && test->max_calling_duration <= ci.totalDuration.sec) {
						LOG(logINFO) <<__FUNCTION__<<"[cancelling:call]["<<call->getId()<<"][test]["<<(ci.role==0?"CALLER":"CALLEE")<<"]["
						     << ci.callIdString <<"]["<<ci.remoteUri<<"]["<<ci.stateText<<"|"<<ci.state<<"]duration["
						     << ci.totalDuration.sec <<">="<<test->max_calling_duration<<"]";
						CallOpParam prm(true);
						try {
							call->hangup(prm);
						} catch (pj::Error e)  {
							if (e.status != 171140) LOG(logERROR) <<__FUNCTION__<<" error :" << e.status;
						}
					}
				} else if (ci.state == PJSIP_INV_STATE_CONFIRMED) {
					std::string res = "call[" + std::to_string(ci.lastStatusCode) + "] reason["+ ci.lastReason +"]";
					call->test->connect_duration = ci.connectDuration.sec;
					call->test->setup_duration = ci.totalDuration.sec - ci.connectDuration.sec;
					call->test->result_cause_code = (int)ci.lastStatusCode;
					call->test->reason = ci.lastReason;
					// check re-invite
					if (call->test->re_invite_interval && ci.connectDuration.sec >= call->test->re_invite_next){
						if (ci.state == PJSIP_INV_STATE_CONFIRMED) {
							CallOpParam prm(true);
							prm.opt.audioCount = 1;
							prm.opt.videoCount = 0;
							LOG(logINFO) <<__FUNCTION__<<" re-invite : call in PJSIP_INV_STATE_CONFIRMED" ;
							try {
								call->reinvite(prm);
								call->test->re_invite_next = call->test->re_invite_next + call->test->re_invite_interval;
							} catch (pj::Error e)  {
								if (e.status != 171140) LOG(logERROR) <<__FUNCTION__<<" error :" << e.status << std::endl;
							}
						}
					}
					// check hangup
					if (call->test->hangup_duration && ci.connectDuration.sec >= call->test->hangup_duration){
						if (ci.state == PJSIP_INV_STATE_CONFIRMED) {
							CallOpParam prm(true);
							LOG(logINFO) << "hangup : call in PJSIP_INV_STATE_CONFIRMED" ;
							try {
								call->hangup(prm);
							} catch (pj::Error e)  {
								if (e.status != 171140) LOG(logERROR) <<__FUNCTION__<<" error :" << e.status << std::endl;
							}
						}
						call->test->update_result();
					}
				}
				if (complete_all || call->test->state == VPT_RUN_WAIT)
					tests_running++;
			}
		}

		int pos=0;
		for (auto test : config->tests_with_rtp_stats) {
			if (test->rtp_stats_ready) {
				test->update_result();
				config->tests_with_rtp_stats.erase(config->tests_with_rtp_stats.begin()+pos);
				LOG(logINFO) << __FUNCTION__ << " erase pos:" << pos;
			} else {
				pos++;
 			}
		}

		if (tests_running > 0) {
			if (status_update) {
				LOG(logINFO) <<__FUNCTION__<<LOG_COLOR_ERROR<<": action[wait] active account tests or call tests in run_wait["<<tests_running<<"] <<<<"<<LOG_COLOR_END;
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
		}
	}
}

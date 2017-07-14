/*
 * Voip Patrol
 * @author Julien Chavanton 2016
 */
#include "voip_patrol.hh"
#define THIS_FILE "voip_patrol.cpp"

using namespace pj;


void get_time_string(char * str_now) {
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	sprintf(str_now,"%02d-%02d-%04d %02d:%02d:%02d", now->tm_mday, now->tm_mon+1, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
}

/* declaration TestCall */
TestCall::TestCall(Account &p_acc, int call_id = PJSUA_INVALID_ID) : Call(p_acc, call_id) {
	test = NULL;
	acc = (TestAccount *)&p_acc;
	recorder_id = -1;
	player_id = -1;
	role = -1; // Caller 0 | callee 1
}

void TestCall::setTest(Test *p_test) {
	test = p_test;
}

void TestCall::onCallTsxState(OnCallTsxStateParam &prm) {
	PJ_UNUSED_ARG(prm);
	std::cout <<"onCallTsxState:id[" << getId() <<"]" << std::endl;
	CallInfo ci = getInfo();
	std::cout << "[CallTsx]: " <<  ci.remoteUri << " [" << ci.stateText << "]" << std::endl;
}

void TestCall::onStreamCreated(OnStreamCreatedParam &prm) {
	std::cout << "[onStreamCreated]\n";
}

static pj_status_t record_call(TestCall* call, pjsua_call_id call_id, const char *caller_contact) {
	pj_status_t status = PJ_SUCCESS;
	pjsua_recorder_id recorder_id;
	char rec_fn[1024] = "voice_ref_files/recording.wav";
	sprintf(rec_fn,"voice_files/%s_rec.wav", caller_contact);
	const pj_str_t rec_file_name = pj_str(rec_fn);
	status = pjsua_recorder_create(&rec_file_name, 0, NULL, -1, 0, &recorder_id);
	if (status != PJ_SUCCESS) {
		std::cout << "[error] tecord_call \n";
		return status;
	}
	call->recorder_id = recorder_id;
	std::cout << "[recorder] created:" << recorder_id << " fn:"<< rec_fn << "\n";
	status = pjsua_conf_connect( pjsua_call_get_conf_port(call_id), pjsua_recorder_get_conf_port(recorder_id) );
}

static pj_status_t stream_to_call(TestCall* call, pjsua_call_id call_id, const char *caller_contact ) {
	pj_status_t status = PJ_SUCCESS;
	pjsua_player_id player_id;
	char fn[] = "voice_ref_files/reference_8000.wav";
	const pj_str_t file_name = pj_str(fn);
	status = pjsua_player_create(&file_name, 0, &player_id);
	if (status != PJ_SUCCESS) {
		std::cout << "[error] play_call \n";
		return status;
	}
	call->player_id = player_id;
	std::cout << "[player] created:" << player_id << "\n";
	status = pjsua_conf_connect( pjsua_player_get_conf_port(player_id), pjsua_call_get_conf_port(call_id) );
}

void TestCall::onCallState(OnCallStateParam &prm) {
	PJ_UNUSED_ARG(prm);

	std::cout <<"onCallState:id[" << getId() <<"]" << std::endl;
	CallInfo ci = getInfo();

	int uri_prefix = 3; // sip:
	std::string remote_user("");
	std::string local_user("");
	std::size_t pos = ci.localUri.find("@");
	if (ci.localUri[0] == '<')
		uri_prefix++;
	if (pos!=std::string::npos) {
		local_user = ci.localUri.substr(uri_prefix, pos - uri_prefix);
	}
	pos = ci.remoteUri.find("@");
	uri_prefix = 3;
	if (ci.remoteUri[0] != '<')
		uri_prefix++;
	if (pos!=std::string::npos) {
		remote_user = ci.remoteUri.substr(uri_prefix, pos - uri_prefix);
	}
	role = ci.role;
	std::cout << "[call]: onCallState: "<< role <<" "<< ci.localUri <<" "<< ci.remoteUri << " [" << ci.stateText <<"|"<< ci.state << "]" << std::endl;

	if (test) {
		test->call_id = getId();
		test->sip_call_id = ci.callIdString;
	}
	if (test && (ci.state == PJSIP_INV_STATE_DISCONNECTED || ci.state == PJSIP_INV_STATE_CONFIRMED)) {
		std::string res = "call[" + std::to_string(ci.lastStatusCode) + "] reason["+ ci.lastReason +"]";
		test->connect_duration = ci.connectDuration.sec;
		test->setup_duration = ci.totalDuration.sec - ci.connectDuration.sec;
		test->result_cause_code = (int)ci.lastStatusCode;
		test->reason = ci.lastReason;
		if(ci.state == PJSIP_INV_STATE_DISCONNECTED || (test->hangup_duration && ci.connectDuration.sec >= test->hangup_duration) ){
			std::cout << "[call] state completed duration: "<< ci.connectDuration.sec << " >= " << test->hangup_duration << std::endl;
			if(!test->completed) {
				test->completed = true;
				if(role == 0 && test->min_mos > 0) {
					test->get_mos();
				}
				test->update_result();
			}
			if(ci.state == PJSIP_INV_STATE_CONFIRMED){
				CallOpParam prm(true);
				std::cout << "hangup : call in PJSIP_INV_STATE_CONFIRMED" << std::endl;
				hangup(prm);
			}
		}
	}
	// Create player and recorder
	if (ci.state == PJSIP_INV_STATE_CONFIRMED){
		stream_to_call(this, ci.id, remote_user.c_str());
		record_call(this, ci.id, remote_user.c_str());
		//pj_thread_sleep(5000);
	}
	if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
		std::cout << "[Call disconnected]\n";
		if(player_id != -1) {
			pjsua_player_destroy(player_id);
			player_id = -1;
		}
		if(recorder_id != -1){
			pjsua_recorder_destroy(recorder_id);
			recorder_id = -1;
		}
	}
}
/* declaration TestCall */


/* declaration TestAccount */
void TestAccount::setTest(Test *ptest) {
	test = ptest;
}

TestAccount::TestAccount() {
	test=NULL;
	config=NULL;
	hangup_duration=0;
	max_duration=0;
	accept_label="-";
}

TestAccount::~TestAccount() {
	std::cout << "[Account] is being deleted: No of calls=" << calls.size() << std::endl;
}

void TestAccount::removeCall(Call *call) {
	for (auto it = calls.begin(); it != calls.end(); ++it) {
		if (*it == call) {
			calls.erase(it);
			break;
		}
	}
}

void TestAccount::onRegState(OnRegStateParam &prm) {
	AccountInfo ai = getInfo();
	std::cout << (ai.regIsActive? "[Register] code:" : "[Unregister] code:") << prm.code << std::endl;
	if(test){
		std::string res = "registration[" + std::to_string(prm.code) + "] reason["+ prm.reason + "] expiration[" + std::to_string(prm.expiration) +"]";
		test->result_cause_code = (int)prm.code;
		test->reason = prm.reason;
		test->completed = true;
		test->update_result();
	}
}

// [onIncomingCall]: <sip:+133663170@fl.gg;isup-oli=00> [NULL]
void TestAccount::onIncomingCall(OnIncomingCallParam &iprm) {
	TestCall *call = new TestCall(*this, iprm.callId);

	CallInfo ci = call->getInfo();
	CallOpParam prm;
	std::cout <<"[onIncomingCall]from["<<ci.remoteUri<<"]to["<<ci.localUri<<"]call-id["<<ci.callIdString<<"]"<< std::endl;
	std::cout <<config<< std::endl;
	if (!call->test) {
		std::cout<<"[onIncomingCall] max call duration["<< hangup_duration <<"]"<<std::endl;
		call->test = new Test(config);
		call->test->hangup_duration = hangup_duration;
		call->test->max_duration = max_duration;
		call->test->expected_cause_code = 200;
		call->test->local_user = ci.localUri;
		call->test->remote_user = ci.remoteUri;
		call->test->type = "accept";
		call->test->label = accept_label;
		call->test->sip_call_id = ci.callIdString;
	}
	calls.push_back(call);
	config->calls.push_back(call);
	prm.statusCode = (pjsip_status_code)200;
	call->answer(prm);
}
/* declaration TestAccount */

/* declaration Test */
Test::Test(Config *p_config){
	char now[20] = {'\0'};
	bool success = false;
	get_time_string(now);
	from="";
	to="";
	type = "unknown";
	completed = false;
	start_time = now;
	config = p_config;
	min_mos = 0.0;
	mos = 0.0;
	expected_cause_code = -1;
	result_cause_code = -1;
	reason = "";
	connect_duration = 0;
	expected_duration = 0;
	setup_duration = 0;
	max_duration = 0;
	hangup_duration = 0;
	call_id = 0;
	sip_call_id = "";
	label = "-";
}

void Test::get_mos() {
	std::string reference = "voice_ref_files/reference_8000.wav";
	std::string degraded = "voice_files/" + remote_user + "_rec.wav";
	mos = pesq_process(8000, reference.c_str(), degraded.c_str());
	std::cout <<"[call] mos["<<mos<<"] min-mos["<<min_mos<<"] "<< reference <<" vs "<< degraded <<"\n";
}

void Test::update_result() {
		char now[20] = {'\0'};
		bool success = false;
		get_time_string(now);
		end_time = now;
		std::string res = "FAILED";
		if (expected_duration && expected_duration != connect_duration) {
			success=false;
		} else if (max_duration && max_duration < connect_duration) {
			success=false;
		} else if(expected_cause_code == result_cause_code && mos >= min_mos) {
			res = "SUCCEED";
			success=true;
		}
		completed = true;

		// preprare result log line
		std::string line ="start["+start_time+"] end["+end_time+"] action["+type+"]"
			          " result["+res+"|"+std::to_string(expected_cause_code)+"] cause_code["+std::to_string(result_cause_code)+"]"
				  " reason["+reason+"] from["+from+"] to["+to+"]";
		config->logFile<<line<<std::endl;
		std::cout<<LOG_COLOR_INFO<<now<<line<<LOG_COLOR_END<<std::endl;

		// prepare HTML report
		std::string td_style= "style='border-color:#98B4E5;border-style:solid;padding:4px;border-width:1px;'";
		std::string td_hd_style = "style='border-color:#98B4E5;background-color: #EEF2F5;border-style:solid;padding:4px;border-width:1px;'";
		if (config->testResults.size() == 0){
			std::string headers = "<tr>"
				              "<td "+td_hd_style+">label</td>"
				              "<td "+td_hd_style+">start</td><td "+td_hd_style+">end</td>"
				              "<td "+td_hd_style+">type</td><td "+td_hd_style+">result</td>"
				              "<td "+td_hd_style+">expected cause code</td><td "+td_hd_style+">result cause code</td><td "+td_hd_style+">reason</td>"
				              "<td "+td_hd_style+">expected mos</td><td "+td_hd_style+">mos</td>"
				              "<td "+td_hd_style+">expected duration</td><td "+td_hd_style+">maximum duration</td>"
				              "<td "+td_hd_style+">hangup duration</td><td "+td_hd_style+">duration</td>"
					      "<td "+td_hd_style+">from</td><td "+td_hd_style+">to</td>\r\n";
			config->testResults.push_back(headers);
		}
		std::string mos_color = "green";
		std::string code_color = "green";
		if (expected_cause_code != result_cause_code)
			code_color = "red";
		if (mos < min_mos)
			mos_color = "red";

		type = type +"["+std::to_string(call_id)+"]<br>"+sip_call_id;
		std::cout << "label["<< label  <<"]\n";
		std::string result = "<tr>"
					 "<td "+td_style+">"+label+"</td>"
			                 "<td "+td_style+">"+start_time+"</td><td "+td_style+">"+end_time+"</td><td "+td_style+">"+type+"</td>"
                                         "<td "+td_style+">"+res+"</td>"
                                         "<td "+td_style+">"+std::to_string(expected_cause_code)+"</td>"
                                         "<td "+td_style+"><font color="+code_color+">"+std::to_string(result_cause_code)+"</font></td>"
                                         "<td "+td_style+">"+reason+"</td>"
                                         "<td "+td_style+">"+std::to_string(min_mos)+"</td><td "+td_style+"><font color="+mos_color+">"+std::to_string(mos)+"</font></td>"
                                         "<td "+td_style+">"+std::to_string(expected_duration)+"</td><td "+td_style+">"+std::to_string(max_duration)+"</td>"
					 "<td "+td_style+">"+std::to_string(hangup_duration)+"</td><td "+td_style+">"+std::to_string(connect_duration)+"</td>"
                                         "<td "+td_style+">"+local_user+"</td>"
                                         "<td "+td_style+">"+remote_user+"</td>"
					 "</tr>\r\n";
		config->testResults.push_back(result);
}
/* declaration Test */


/* declaration Config */
Config::Config(){
}

void Config::update_result(std::string text){
	char now[20] = {'\0'};
	get_time_string(now);
	logFile<<now<<": "<<text<<"\n";
}
Config::~Config() {
	logFile.close();
}

bool Config::wait(){
	bool completed = false;
	int tests_running = 0;
	bool status_update = true;
	while (!completed) {
		for (auto & account : accounts) {
			AccountInfo acc_inf = account->getInfo();
			if (account->test && account->test->completed){
				std::cout << "delete account test["<<account->test<<"]\n";
				delete account->test;
				account->test = NULL;
			} else if (account->test) {
				tests_running++;
			}
		}
		for (auto & call : calls) {
			if (call->test && call->test->completed){
				std::cout << "delete call test["<<call->test<<"]\n";
				delete call->test;
				call->test = NULL;
			} else if (call->test) {
				CallInfo ci = call->getInfo();
				std::cout <<"[wait:call]["<<call->getId()<<"][test]["<<(ci.role==0?"CALLER":"CALLEE")<<"]: "<<ci.remoteUri<<" ["<<ci.stateText<<"|"<<ci.state<<"] duration["<<ci.connectDuration.sec<<">="<<call->test->expected_duration<<"]"<<std::endl;
				if (ci.state == PJSIP_INV_STATE_CONFIRMED) {
					std::string res = "call[" + std::to_string(ci.lastStatusCode) + "] reason["+ ci.lastReason +"]";
					call->test->connect_duration = ci.connectDuration.sec;
					call->test->setup_duration = ci.totalDuration.sec - ci.connectDuration.sec;
					call->test->result_cause_code = (int)ci.lastStatusCode;
					call->test->reason = ci.lastReason;
					if(call->test->hangup_duration && ci.connectDuration.sec >= call->test->hangup_duration){
						if(ci.state == PJSIP_INV_STATE_CONFIRMED) {
							CallOpParam prm(true);
							std::cout << "hangup : call in PJSIP_INV_STATE_CONFIRMED" << std::endl;
							call->hangup(prm);
						}
						call->test->completed = true;
						if(call->role == 0 && call->test->min_mos > 0) {
							call->test->get_mos();
						}
						call->test->update_result();
					}
				}
				tests_running++;
			}
		}
		if(tests_running > 0){
			if (status_update) {
				std::cout << "waiting for tests completion active tests["<<tests_running<<"]...\n";
				status_update = false;
			}
			tests_running=0;
			pj_thread_sleep(1000);
		} else {
			pj_thread_sleep(1000);
			std::cout << "action[wait] completed\n";
			update_result("action[wait] completed");
			completed = true;
		}
	}
}

bool Config::process(std::string p_configFileName, std::string p_logFileName) {
	const char* tag = "[loading xml config] ";
	// config loader
	ezxml_t xml_actions, xml_action;
	configFileName = p_configFileName;
	ezxml_t xml_conf = ezxml_parse_file(configFileName.c_str());
	xml_conf_head = xml_conf; // saving the head if the linked list
	logFileName = p_logFileName;

	logFile.open (logFileName.c_str(), std::fstream::in | std::fstream::out | std::fstream::app);
	if(logFile.is_open()) {
		std::cout << "open log file:" << configFileName << "\n";
	} else {
		std::cerr <<tag<< "[error] test can not open log file :" << logFileName << std::endl;
		return false;
	}

	if(!xml_conf){
		std::cerr <<tag<< "[error] test can not load file :" << configFileName << std::endl;
		return false;
	} else {
		update_result("loading tests...");
	}

	for (xml_actions = ezxml_child(xml_conf, "actions"); xml_actions; xml_actions=xml_actions->next){
		std::cout <<tag<< xml_actions->name << std::endl;
		for (xml_action = ezxml_child(xml_actions, "action"); xml_action; xml_action=xml_action->next){
			if (!ezxml_attr(xml_action,"type")) {
				std::cerr <<" >> "<<tag<<"invalid action !\n";
				continue;
			}
			string action_type = ezxml_attr(xml_action,"type");
			std::cout <<" >> "<<tag<<"type:"<< action_type << std::endl;
			/* action */
			if ( action_type.compare("wait") == 0 ) {
				wait();
			} else if ( action_type.compare("alert") == 0 ) {
				if (!ezxml_attr(xml_action,"email")) {
					std::cerr <<" >> "<<tag<<"missing pamameter !\n";
					continue;
				}
				this->alert_email_to = ezxml_attr(xml_action,"email");
				if (ezxml_attr(xml_action,"email_from")) this->alert_email_from = ezxml_attr(xml_action,"email_from");
				if (ezxml_attr(xml_action,"smtp_host")) this->alert_server_url = ezxml_attr(xml_action,"smtp_host");
			} else if ( action_type.compare("register") == 0 ) {
				if (!ezxml_attr(xml_action,"username") || !ezxml_attr(xml_action,"realm") || !ezxml_attr(xml_action,"password") || !ezxml_attr(xml_action,"registrar")) {
					std::cerr <<" >> "<<tag<<"missing pamameter !\n";
					continue;
				}
				std::string username = ezxml_attr(xml_action,"username");
				TestAccount * acc = NULL;


				for (auto & acc : accounts) {
					AccountInfo acc_inf = acc->getInfo();
					std::cout << "[register]["<<acc_inf.uri<<"]<>["<<username<<"]"<<std::endl;
					if( acc_inf.uri.compare(4,username.length(),username) == 0 ){
						std::cout << "found account id["<< acc_inf.id <<"] uri[" << acc_inf.uri <<"] active["<<acc_inf.regIsActive<<"]"<< std::endl;
						break;
					}
				}
				if (acc) {
					AccountInfo acc_inf = acc->getInfo();
					std::cout << "found: " << username <<" [unregistering]"<< std::endl;
					acc->setRegistration(false);
					while (acc_inf.regIsActive) {
						pj_thread_sleep(500);
						acc_inf = acc->getInfo();
					}
					std::cout << "found: " << username <<" [unregistered]"<< std::endl;
				} else {
					acc = new TestAccount();
					accounts.push_back(acc);
				}

				Test *test = new Test(this);
				test->local_user = username;
				test->remote_user = username;
				std::string password = ezxml_attr(xml_action,"password");
				std::string registrar = ezxml_attr(xml_action,"registrar");
				test->expected_cause_code = atoi(ezxml_attr(xml_action,"expected_cause_code"));
				test->from = username;
				test->type = action_type;

				std::cout <<" >> "<<tag<< "sip:" + username + "@" + registrar  << std::endl;
				// register account
				AccountConfig acc_cfg;
				acc_cfg.idUri = "sip:" + username + "@" + registrar;
				acc_cfg.regConfig.registrarUri = "sip:" + registrar;
				acc_cfg.sipConfig.transportId = transport_id_udp;
				if (ezxml_attr(xml_action,"transport")) {
					std::string transport = ezxml_attr(xml_action,"transport");
					if (transport.compare("tcp") == 0) {
						acc_cfg.sipConfig.transportId = transport_id_tcp;
					}
				}
				acc_cfg.sipConfig.authCreds.push_back( AuthCredInfo("digest", ezxml_attr(xml_action,"realm"), username, 0, password) );

				acc->config = this;
				acc->create(acc_cfg);
				acc->setTest(test);

			} else if ( action_type.compare("accept") == 0 ) {
				if (!ezxml_attr(xml_action,"callee") ) {
					std::cerr <<" >> "<<tag<<"missing action parameters for " << action_type << std::endl;
					continue;
				}
				TestAccount * acc = NULL;
				std::string callee = ezxml_attr(xml_action,"callee");
				for (auto & acc : accounts) {
					AccountInfo acc_inf = acc->getInfo();
					std::cout << "[accept]["<<acc_inf.uri<<"]<>["<<callee<<"]"<<std::endl;
					if( acc_inf.uri.compare(4,callee.length(),callee) == 0 ){
						std::cout << "found callee account id["<< acc_inf.id <<"] uri[" << acc_inf.uri <<"]"<< std::endl;
					}
				}
				if (!acc) {
					std::cout << "account not found: " << callee << std::endl;
					continue;
				}
				if (ezxml_attr(xml_action,"hangup")){
					acc->hangup_duration = atoi(ezxml_attr(xml_action,"hangup"));
				} else {
					acc->hangup_duration = 0;
				}
				if (ezxml_attr(xml_action,"max_duration")){
					acc->max_duration = atoi(ezxml_attr(xml_action,"max_duration"));
				} else {
					acc->max_duration = 0;
				}
				if (ezxml_attr(xml_action,"label")){
					acc->accept_label = ezxml_attr(xml_action,"label");
				}
			} else if ( action_type.compare("call") == 0 ) {
				if (!ezxml_attr(xml_action,"caller") || !ezxml_attr(xml_action,"callee") ) {
					std::cerr <<" >> "<<tag<<"missing action parameters for " << action_type << std::endl;
					continue;
				}

				std::string caller = ezxml_attr(xml_action,"caller");
				std::string callee = ezxml_attr(xml_action,"callee");

				std::cerr <<" >> "<<tag<<"action parameters found : " << action_type << std::endl;
				// make call begin
				TestAccount * acc = NULL;
				for (auto & acc : accounts) {
					AccountInfo acc_inf = acc->getInfo();
					if( acc_inf.uri.compare(4,string::npos,caller) == 0 ){
						std::cout << "found caller account id["<< acc_inf.id <<"] uri[" << acc_inf.uri <<"]"<< std::endl;
					}
				}
				if (!acc) {
					std::cout << "caller not found: " << caller << std::endl;
					continue;
				}
				Test *test = new Test(this);
				std::size_t pos = caller.find("@");
				if (pos!=std::string::npos) {
					test->local_user = caller.substr(0, pos);
				}
				pos = callee.find("@");
				if (pos!=std::string::npos) {
					test->remote_user = callee.substr(0, pos);
				}
				TestCall *call = new TestCall(*acc);
				calls.push_back(call);

				if (ezxml_attr(xml_action,"mos")){
					test->min_mos = atof(ezxml_attr(xml_action,"mos"));
				}
				if (ezxml_attr(xml_action,"duration")){
					test->expected_duration = atoi(ezxml_attr(xml_action,"duration"));
				}
				if (ezxml_attr(xml_action,"max_duration")){
					test->max_duration = atoi(ezxml_attr(xml_action,"max_duration"));
				}
				if ( ezxml_attr(xml_action,"hangup") ) {
					test->hangup_duration = atoi(ezxml_attr(xml_action,"hangup"));
				}
				if (ezxml_attr(xml_action,"label")){
					test->label = ezxml_attr(xml_action,"label");
				}
				call->test = test;
				test->expected_cause_code = atoi(ezxml_attr(xml_action,"expected_cause_code"));
				test->from = caller;
				test->to = callee;
				test->type = action_type;
				acc->calls.push_back(call);
				CallOpParam prm(true);
				prm.opt.audioCount = 1;
				prm.opt.videoCount = 0;
				std::cout << "call->test:" << test << " " << call->test->type  << "\n";
				std::cout << "calling :" +callee+ "\n" ;
				call->makeCall("sip:"+callee, prm);
				// make call end
			} else {
				std::cerr <<" >> "<<tag<<"unknown action !\n";
			}
		}
	}
}
/* declaration Config */

/* declaration Alert */
Alert::Alert(Config * p_config){
	curl = curl_easy_init();
	config = p_config;
}
void Alert::prepare(void){
//	std::string date = "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n";
//	upload_data.payload_content.push_back(date);
	alert_server_url = config->alert_server_url;
	std::string to = "To: <"+config->alert_email_to+">\r\n";
	upload_data.payload_content.push_back(to);
	std::string from = "From: <"+config->alert_email_from+">\r\n";
	upload_data.payload_content.push_back(from);
	std::string messageId = "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>\r\n";
	upload_data.payload_content.push_back(messageId);
	std::string subject = "Subject: VoIP Patrol test report\r\n";
	upload_data.payload_content.push_back(subject);
	std::string content_type = "Content-type: text/html\r\n";
	upload_data.payload_content.push_back(content_type);
	std::string bodySeparator = "\r\n";
	upload_data.payload_content.push_back(bodySeparator);

	std::string tb_style = "style='font-size:8pt;font-family:\"DejaVu Sans\",Verdana;"
                                       "border-collapse:collapse;border-spacing:0px;"
                                       "border-style:solid;border-width:1px;text-align:center;'";
	std::string html_start = "<html><table "+tb_style+">";
	upload_data.payload_content.push_back(html_start);
	for (auto & testResult : config->testResults) {
		upload_data.payload_content.push_back(testResult);
	}
	std::string html_end = "</table></html>\n\r";
	upload_data.payload_content.push_back(html_end);
}

void Alert::send(void) {
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
	upload_data.lines_read = 0;
	if (config->alert_server_url.empty() || config->alert_email_to.empty() || config->alert_email_from.empty())
		return;
	prepare();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, alert_server_url.c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, alert_email_from.c_str());
		recipients = curl_slist_append(recipients, config->alert_email_to.c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, &Alert::payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_data);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		std::cerr << "curl_easy_perform() failed: "  << curl_easy_strerror(res) << alert_server_url <<"\n";
		curl_slist_free_all(recipients);
		curl_easy_cleanup(curl);
	}
	std::cout << "email alert sent...\n";
}

size_t Alert::payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
	upload_data_t *upload_data = (upload_data_t *)userp;
	const char *data;

	if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	if(upload_data->lines_read >= upload_data->payload_content.size())
		return 0;
//	data = payload_text[upload_data->lines_read];
	data = upload_data->payload_content[upload_data->lines_read].c_str();
	std::cout<<LOG_COLOR_INFO<<data<<LOG_COLOR_END;
	if(data) {
		size_t len = strlen(data);
		memcpy(ptr, data, len);
		upload_data->lines_read++;
		return len;
	}

	return 0;
}
/* declaration Alert */

int main(int argc, char **argv){
	int ret = 0;
	Endpoint ep;
	Config config;
	std::string conf_fn = "conf.xml";
	std::string log_fn = "tests.log";

	// command line argument
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if ((arg == "-h") || (arg == "--help")) {
			std::cout << argv[0] << "\n -c,--conf <conf.xml> \n -l,--log <test.log>\n" << std::endl;
			return 0;
		} else if( (arg == "-c") || (arg == "--conf") ) {
			if (i + 1 < argc) {
				conf_fn = argv[++i];
			}
		} else if( (arg == "-l") || (arg == "--log")) {
			if (i + 1 < argc) {
				log_fn = argv[++i];
			}
		}
	}

	try {
		ep.libCreate();
		// Init library
		EpConfig ep_cfg;
		ep_cfg.logConfig.level = 4;
		ep_cfg.logConfig.consoleLevel = 0;
		ep_cfg.logConfig.filename = "pjsua.log";
		ep_cfg.medConfig.ecTailLen = 0; // disable echo canceller
		ep_cfg.medConfig.noVad = 1;

		ep.libInit( ep_cfg );
		// pjsua_set_null_snd_dev() before calling pjsua_start().

		// Transport
		TransportConfig tcfg;
		tcfg.port = 5061;
		config.transport_id_tcp = ep.transportCreate(PJSIP_TRANSPORT_TCP, tcfg);
		config.transport_id_udp = ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);


		// load config and execute test
		pjsua_set_null_snd_dev();
		ep.libStart();
		config.process(conf_fn, log_fn);
		config.wait();

		std::cout << "checking alerts..." << std::endl;

		// send email reporting
		Alert alert(&config);
		alert.send();

		std::cout << "hangup all calls..." << std::endl;
		ep.hangupAllCalls();

		ret = PJ_SUCCESS;
	} catch (Error & err) {
		std::cout << "Exception: " << err.info() << std::endl;
		ret = 1;
	}

	try {
		ep.libDestroy();
	} catch(Error &err) {
		// std::cout << "Exception: " << err.info() << std::endl;
		ret = 1;
	}

	if (ret == PJ_SUCCESS) {
		std::cout << "Success" << std::endl;
	} else {
		std::cout << "Error Found" << std::endl;
	}

	std::cout << "exiting !" << std::endl;
	return ret;
}



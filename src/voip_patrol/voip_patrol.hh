/*
 * Voip Patrol
 * @author Julien Chavanton 2016
 */
#ifndef VOIP_PATROL_H
#define VOIP_PATROL_H
#include "action.hh"
#include <pjsua2.hpp>
#include <iostream>
#include <fstream>
#include <memory>
#include <pj/file_access.h>
#include "ezxml/ezxml.h"
#include "curl/email.h"
#include "patrol_pesq.h"
#include <sstream>
#include <ctime>
#include "log.h"
#include "version.h"

#define LOG_COLOR_INFO "\e[1;35m"
#define LOG_COLOR_ERROR "\e[1;31m"
#define LOG_COLOR_END "\e[0m\n"

using namespace pj;

class TestAccount;
class TestCall;
class Test;
class Config;

typedef struct upload_data {
	int lines_read;
	std::vector<std::string> payload_content;
} upload_data_t;


class Alert {
	public:
		Alert(Config* config);
		void prepare(void);
		std::string alert_email_to;
		std::string alert_email_from;
		std::string alert_server_url;
		void send(void);
		static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp);
		upload_data_t upload_data;
		Config * config;
	private:
		CURL *curl;

};


class ResultFile {
	public:
		ResultFile(std::string file_name);
		void flush();
		bool open();
		void close();
		bool write(std::string res);
	private:
		std::fstream file;
		std::string name;
};

class Config {
	public:
		Config(std::string result_file_name);
		~Config();
		void log(std::string message);
		bool process(std::string ConfigFileName, std::string jsonResultFile);
		bool wait(bool complete_all);
		TestAccount* findAccount(std::string);
		std::vector<TestAccount *> accounts;
		std::vector<TestCall *> calls;
		std::vector<Test *> tests;
		std::vector<std::string> testResults;
		ezxml_t xml_conf_head;
		ezxml_t xml_test;
		void update_result(std::string text);
		void removeCall(TestCall *call);
		std::string alert_email_to;
		std::string alert_email_from;
		std::string alert_server_url;
		TransportId transport_id_udp;
		TransportId transport_id_tcp;
		TransportId transport_id_tls;
		int json_result_count;
		Action action;
		ResultFile result_file;
	private:
		std::string configFileName;
};

typedef enum call_wait_state {
	INV_STATE_NULL,        //0 Before INVITE is sent or received
	INV_STATE_CALLING,     //1 After INVITE is sent
	INV_STATE_INCOMING,    //2 After INVITE is received.
	INV_STATE_EARLY,       //3 After response with To tag.
	INV_STATE_CONNECTING,  //4 After 2xx is sent/received.
	INV_STATE_CONFIRMED,   //5 After ACK is sent/received.
	INV_STATE_DISCONNECTED
} call_wait_state_t;

typedef enum test_run_state {
	VPT_RUN,              // test is running
	VPT_RUN_WAIT,         // test is running and will block execution when command wait is used
	VPT_DONE              // test is completed
} test_state_t;

class Test {
	public:
		Test(Config *config);
		std::string type;
		void update_result(void);
		std::string from;
		std::string to;
		int expected_cause_code;
		int result_cause_code;
		bool completed;
		std::string start_time;
		std::string end_time;
		float min_mos;
		float mos;
		std::string reason;
		int connect_duration;
		int hangup_duration;
		int setup_duration;
		int expected_duration;
		int max_duration;
		int max_calling_duration;
		void get_mos();
		std::string local_user;
		std::string remote_user;
		std::string call_direction;
		std::string sip_call_id;
		std::string label;
		std::string transport;
		std::string peer_socket;
		call_wait_state_t wait_state;
		test_state_t state;
		int call_id;
		bool recording;
		bool playing;

	private:
		Config *config;
};

class TestAccount : public Account {
	public:
		std::vector<TestCall *> calls;
		Test *test;
		Config *config;
		void setTest(Test *test);
		TestAccount();
		~TestAccount();
		void removeCall(Call *call);
		virtual void onRegState(OnRegStateParam &prm);
		virtual void onIncomingCall(OnIncomingCallParam &iprm);
		int hangup_duration;
		int max_duration;
		std::string accept_label;
};

class TestCall : public Call {
	public:
		TestCall(TestAccount *acc, int call_id=PJSUA_INVALID_ID);
		~TestCall();
		Test *test;
		void setTest(Test *test);
		virtual void onCallTsxState(OnCallTsxStateParam &prm);
		virtual void onCallState(OnCallStateParam &prm);
		virtual void onStreamCreated(OnStreamCreatedParam &prm);
		virtual void onStreamDestroyed(OnStreamDestroyedParam &prm);
		pjsua_recorder_id recorder_id;
		pjsua_player_id player_id;
		int role;
	private:
		TestAccount *acc;

};


#endif

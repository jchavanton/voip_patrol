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

#ifndef VOIP_PATROL_H
#define VOIP_PATROL_H
#include "action.hh"
#include <pjsua2.hpp>
#include <iostream>
#include <fstream>
#include <memory>
#include <mutex>
#include <pj/file_access.h>
#include "ezxml/ezxml.h"
#include "curl/email.h"
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
		std::string name;
	private:
		std::fstream file;
};


class Config {
	public:
		Config(std::string result_file_name);
		~Config();
		void log(std::string message);
		bool process(std::string ConfigFileName, std::string jsonResultFile);
		bool wait(bool complete_all);
		TestAccount* findAccount(std::string);
		TestAccount* createAccount(AccountConfig acc_cfg);
		void createDefaultAccount();
		std::vector<TestAccount *> accounts;
		std::vector<TestCall *> calls;
		std::vector<Test *> tests;
		std::vector<std::string> testResults;
		ezxml_t xml_conf_head;
		ezxml_t xml_test;
		void set_output_file(std::string);
		bool removeCall(TestCall *call);
		std::string alert_email_to;
		std::string alert_email_from;
		std::string alert_server_url;
		TransportId transport_id_udp;
		TransportId transport_id_tcp;
		TransportId transport_id_tls;
		int json_result_count;
		Action action;
		ResultFile result_file;
		struct {
			string ca_list;
			string private_key;
			string certificate;
			int verify_server;
			int verify_client;
		} tls_cfg;
		std::vector<Test *> tests_with_rtp_stats;
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
} call_state_t;

call_state_t get_call_state_from_string (string state);
string get_call_state_string (call_state_t state);

const char default_playback_file[] = "voice_ref_files/reference_8000.wav";

typedef enum test_run_state {
	VPT_RUN,              // test is running
	VPT_RUN_WAIT,         // test is running and will block execution when command wait is used
	VPT_DONE              // test is completed
} test_state_t;

class Test {
	public:
		Test(Config *config, string type);
		std::string type;
		void update_result(void);
		std::string from;
		std::string to;
		int expected_cause_code;
		pjsip_status_code code;
		int result_cause_code;
		bool completed;
		std::string start_time;
		std::string end_time;
		float min_mos;
		bool rtp_stats;
		float mos;
		std::string reason;
		int connect_duration;
		int hangup_duration;
		int setup_duration;
		int expected_duration;
		int max_duration;
		int ring_duration;
		int max_calling_duration;
		void get_mos();
		std::string local_user;
		std::string remote_user;
		std::string call_direction;
		std::string sip_call_id;
		std::string label;
		std::string transport;
		std::string peer_socket;
		std::string dtmf_recv;
		call_state_t wait_state;
		test_state_t state;
		int call_id;
		bool recording;
		bool playing;
		string record_fn;
		string reference_fn;
		string rtp_stats_json;
		string play;
		string play_dtmf;
		bool rtp_stats_ready;
		bool queued;
	private:
		Config *config;
		std::mutex process_result;
};

class VoipPatrolEnpoint : public Endpoint {
	public:
		Config *config;
	private:
		void onSelectAccount(OnSelectAccountParam &param);
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
		int ring_duration;
		int response_delay;
		bool rtp_stats;
		bool early_media;
		SipHeaderVector x_headers;
		string play;
		string play_dtmf;
		string timer;
		call_state_t wait_state;
		std::string accept_label;
		string reason;
		int code;
};

class TestCall : public Call {
	public:
		TestCall(TestAccount *acc, int call_id=PJSUA_INVALID_ID);
		~TestCall();
		Test *test;
		void setTest(Test *test);
		virtual void onCallRxOffer(OnCallTsxStateParam &prm);
		virtual void onCallTsxState(OnCallTsxStateParam &prm);
		virtual void onCallState(OnCallStateParam &prm);
		virtual void onStreamCreated(OnStreamCreatedParam &prm);
		virtual void onStreamDestroyed(OnStreamDestroyedParam &prm);
		virtual void onDtmfDigit(OnDtmfDigitParam &prm);
		void makeCall(const string &dst_uri, const CallOpParam &prm, const string &to_uri) throw(Error);
		pjsua_recorder_id recorder_id;
		pjsua_player_id player_id;
		int role;
		int rtt;
	private:
		TestAccount *acc;

};


#endif

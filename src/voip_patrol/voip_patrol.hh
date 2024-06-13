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
#include <vector>
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

struct sipLatency {
	int invite100Ms;
	int invite18xMs;
	int invite200Ms;
	int bye200Ms;
	pj_time_val inviteSentTs;
	pj_time_val byeSentTs;
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

class VoipPatrolEnpoint : public Endpoint {
	public:
		Config *config;
		void setCodecs(string &list, int priority);
	private:
		void onSelectAccount(OnSelectAccountParam &param);
};

typedef struct turn_config {
	bool enabled;
	bool stun_only;
	bool sip_stun_use;
	bool media_stun_use;
	std::string server;
	std::string username;
	std::string password;
	bool password_hashed;
} turn_config_t;

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
		turn_config_t turn_config;
		std::vector<TestAccount *> accounts;
		std::vector<TestCall *> calls;
		std::vector<TestCall *> new_calls;
		std::vector<Test *> tests;
		std::vector<std::string> testResults;
		ezxml_t xml_conf_head;
		ezxml_t xml_test;
		void set_output_file(std::string);
		bool removeCall(TestCall *call);
		bool graceful_shutdown;
		bool rewrite_ack_transport;
		std::string alert_email_to;
		std::string alert_email_from;
		std::string alert_server_url;
		TransportId transport_id_udp{-1};
		TransportId transport_id_tcp{-1};
		TransportId transport_id_tls{-1};
		Action action;
		ResultFile result_file;
		std::mutex checking_calls;
		std::mutex new_calls_lock;
		struct {
			string ca_list;
			string private_key;
			string certificate;
			int verify_server;
			int verify_client;
		} tls_cfg;
		struct {
			string public_address{};
			string bound_address{};
		} ip_cfg;
		struct {
			int port;
			int port_range;
		} rtp_cfg;
		std::vector<Test *> tests_with_rtp_stats;
		VoipPatrolEnpoint *ep;
		std::mutex process_result;
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
		std::string from{""};
		std::string to{""};
		int expected_cause_code{-1};
		pjsip_status_code code;
		int result_cause_code{-1};
		bool completed{false};
		std::string start_time;
		std::string end_time;
		float min_mos{0.0};
		float mos{0.0};
		bool rtp_stats{false};
		bool late_start{false};
		bool record_early{false};
		bool record{false};
		bool tone_detected{false};
		string force_contact{""};
		std::string reason{""};
		int connect_duration{0};
		int hangup_duration{0};
		int re_invite_next{0};
		int re_invite_interval{0};
		int setup_duration{0};
		int expected_duration{0};
		int max_duration{0};
		int ring_duration{0};
		int rtp_stats_count{0};
		int max_ringing_duration{0};
		int response_delay{0};
		int cancel{0};
		void get_mos();
		string expected_message;
		string message;
		std::string local_user;
		std::string local_uri;
		std::string local_contact;
		std::string remote_user;
		std::string remote_uri;
		std::string remote_contact;
		std::string call_direction;
		std::string sip_call_id{""};
		std::string label{"-"};
		std::string transport;
		std::string peer_socket;
		std::string dtmf_recv;
		call_state_t wait_state{INV_STATE_NULL};
		test_state_t state{VPT_RUN};
		int call_id{0};
		bool recording{false};
		bool playing{false};
		bool early_media {false};
		bool is_recording_running {false};
		string record_fn;
		string reference_fn;
		string rtp_stats_json;
		string play;
		string play_dtmf;
		string srtp;
		bool rtp_stats_ready{false};
		bool queued{false};
		vector<ActionCheck> checks;
		Config *config;
		sipLatency sip_latency;
};

class TestAccount : public Account {
	public:
		std::vector<TestCall *> calls;
		Test *test;
		Test *testAccept;
		Config *config;
		void setTest(Test *test);
		TestAccount();
		~TestAccount();
		void removeCall(Call *call);
		virtual void onRegState(OnRegStateParam &prm);
		virtual void onIncomingCall(OnIncomingCallParam &iprm);
		virtual void onInstantMessage(OnInstantMessageParam &prm);
		virtual void onInstantMessageStatus(OnInstantMessageStatusParam &prm);
		int hangup_duration {0};
		int re_invite_interval {0};
		int max_duration {0};
		int ring_duration {0};
		int response_delay {0};
		bool rtp_stats {false};
		bool late_start {false};
		bool record_early {false};
		bool record {false};
		bool unregistering {false};
		string force_contact;
		bool early_media {false};
		SipHeaderVector x_headers;
		int call_count {-1};
		int message_count {-1};
		string play;
		string play_dtmf;
		string timer;
		string srtp;
		call_state_t wait_state;
		std::string accept_label;
		string reason;
		int code;
		vector<ActionCheck> checks;
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
		virtual void onCallMediaState(OnCallMediaStateParam &prm);
		virtual void onCallMediaUpdate(OnCallMediaStateParam &prm);
		virtual void onStreamDestroyed(OnStreamDestroyedParam &prm);
		virtual void onDtmfDigit(OnDtmfDigitParam &prm);
		void makeCall(const string &dst_uri, const CallOpParam &prm, const string &to_uri);
		void hangup(const CallOpParam &prm);
		pjsua_recorder_id recorder_id{-1};
		pjsua_recorder_id tone_detector_id{-1};
		pjsua_player_id player_id{-1};
		int role;
		int rtt;
		int durationBeforeEarly;
		bool is_disconnecting(){return disconnecting;};
		TestAccount *acc;
	private:
		bool disconnecting;
};

#endif

/*
 * Voip Patrol
 * @author Julien Chavanton 2016
 */
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

#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

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

class Config {
	public:
		Config();
		~Config();
		bool process(std::string ConfigFileName, std::string logFileName);
		bool wait();
		std::vector<TestAccount *> accounts;
		std::vector<TestCall *> calls;
		std::vector<Test *> tests;
		std::fstream logFile;
		std::vector<std::string> testResults;
		ezxml_t xml_conf_head;
		ezxml_t xml_test;
		void update_result(std::string text);
	private:
		std::string configFileName;
		std::string logFileName;
};

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
		int setup_duration;
		int expected_duration;
		void get_mos();
		std::string local_user;
		std::string remote_user;
	private:
		Config *config;
};

class TestAccount : public Account {
	public:
		std::vector<Call *> calls;
		Test *test;
		void setTest(Test *test);
		TestAccount();
		~TestAccount();
		void removeCall(Call *call);
		virtual void onRegState(OnRegStateParam &prm);
		virtual void onIncomingCall(OnIncomingCallParam &iprm);
};

class TestCall : public Call {
	public:
		TestCall(Account &acc, int call_id);
		Test *test;
		void setTest(Test *test);
		virtual void onCallTsxState(OnCallTsxStateParam &prm);
		virtual void onCallState(OnCallStateParam &prm);
		virtual void onStreamCreated(OnStreamCreatedParam &prm);
		pjsua_recorder_id recorder_id;
		pjsua_player_id player_id;
		std::string local_user;
		std::string remote_user;
		int role;
	private:
		TestAccount *acc;

};

/*
 * Voip Patrol
 * @author Julien Chavanton 2018
 */

#ifndef VOIP_PATROL_ACTION_H
#define VOIP_PATROL_ACTION_H

#include "voip_patrol.hh"
#include <iostream>
#include <vector>

class Config;

using namespace std;

enum class APType { apt_integer, apt_string, apt_float };

struct ActionParam {
	ActionParam(string name, bool required, APType type, string s_val="", int i_val=0, float f_val=0.0)
                 : type(type), required(required), name(name), i_val(i_val), s_val(s_val), f_val(f_val) {}
	APType type {APType::apt_integer};
	string name;
	int i_val;
	string s_val;
	float f_val;
	bool required;
};

class Action {
	public:
			Action(Config *cfg);
			vector<ActionParam>* get_params(string);
			bool set_param(ActionParam&, const char *);
			void do_call(vector<ActionParam> &params);
			void do_accept(vector<ActionParam> &params);
			void do_wait(vector<ActionParam> &params);
			void do_register(vector<ActionParam> &params);
			void set_config(Config *);
			Config* get_config();
	private:
			string get_env(string);
			void init_actions_params();
			vector<ActionParam> do_call_params;
			vector<ActionParam> do_register_params;
			vector<ActionParam> do_wait_params;
			vector<ActionParam> do_accept_params;
			Config* config;
};

#endif

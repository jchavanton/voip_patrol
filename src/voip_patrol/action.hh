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

#ifndef VOIP_PATROL_ACTION_H
#define VOIP_PATROL_ACTION_H

#include "voip_patrol.hh"
#include "check.hh"
#include <pjsua2.hpp>

class Config;
class ActionCheck;

using namespace std;

enum class APType { apt_integer, apt_string, apt_float, apt_bool };

struct ActionParam {
	ActionParam(string name, bool required, APType type, string s_val="", int i_val=0, float f_val=0.0, bool b_val=false)
                 : type(type), required(required), name(name), i_val(i_val), s_val(s_val), f_val(f_val) , b_val(b_val) {}
	APType type {APType::apt_integer};
	string name;
	int i_val;
	string s_val;
	float f_val;
	bool b_val;
	bool required;
};


class Action {
	public:
			Action(Config *cfg);
			vector<ActionParam> get_params(string);
			bool set_param(ActionParam&, const char *val);
			bool set_param_by_name(vector<ActionParam> *params, const string name, const char *val=nullptr);
			void do_call(vector<ActionParam> &params, vector<ActionCheck> &checks, pj::SipHeaderVector &x_headers);
			void do_accept(vector<ActionParam> &params, vector<ActionCheck> &checks, pj::SipHeaderVector &x_headers);
			void do_wait(vector<ActionParam> &params);
			void do_register(vector<ActionParam> &params, vector<ActionCheck> &checks, pj::SipHeaderVector &x_headers);
			void do_alert(vector<ActionParam> &params);
			void do_codec(vector<ActionParam> &params);
			void do_turn(vector<ActionParam> &params);
			void do_transport(vector<ActionParam> &params);
			void set_config(Config *);
			Config* get_config();
	private:
			string get_env(string);
			void init_actions_params();
			vector<ActionParam> do_call_params;
			vector<ActionParam> do_register_params;
			vector<ActionParam> do_wait_params;
			vector<ActionParam> do_accept_params;
			vector<ActionParam> do_alert_params;
			vector<ActionParam> do_codec_params;
			vector<ActionParam> do_turn_params;
			vector<ActionParam> do_transport_params;
			Config* config;
};

#endif

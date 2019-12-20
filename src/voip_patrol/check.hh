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

#ifndef VOIP_PATROL_CHECK_H
#define VOIP_PATROL_CHECK_H

#include "voip_patrol.hh"
#include <pjsua2.hpp>

using namespace std;

class ActionCheck {
	public:
		pj::SipHeader hdr;
		string regex;
		string method;
		int code{0};
		bool result{false};
};

bool check_regex(string m, string e);
void check_checks(vector<ActionCheck> &checks, pjsip_msg* msg, string message);

#endif

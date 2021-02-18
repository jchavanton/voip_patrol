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
#include "check.hh"
#include "pj_util.hpp"
#include <regex>



bool check_regex(string m, string e) {
	std::istringstream ss(m);
	std::string line;
	std::regex re(e);
	while (std::getline(ss, line)) {
		line.pop_back();
		LOG(logINFO) <<__FUNCTION__<<">>> "<< line;
		if (std::regex_match(line, re)) {
			LOG(logINFO) <<__FUNCTION__<<": matching ! ["<<e<<"]";
			return true;
		}
	}
	LOG(logINFO) <<__FUNCTION__<<": not matching ! ["<<e<<"]";
	return false;
}


void check_checks(vector<ActionCheck> &checks, pjsip_msg* msg, string message) {
	std::string method = pj2Str(msg->line.req.method.name);
	LOG(logINFO) <<__FUNCTION__<<": "+ method;
	// action checks for headers
	for (vector<ActionCheck> :: iterator check = checks.begin(); check != checks.end(); ++check){
		if (!check->regex.empty()) {
			if (method != check->method) continue;
			if (check_regex(message, check->regex)) {
				check->result = true;
			}
			continue;
		}
		if (method != check->method) continue;
		if (check->hdr.hName == "") continue;
		LOG(logINFO) <<__FUNCTION__<<" check-header:"<< check->hdr.hName<<" "<<check->hdr.hValue;
		pj_str_t header_name;
		header_name.slen = strlen(check->hdr.hName.c_str());
		header_name.ptr = (char*) check->hdr.hName.c_str();

		pjsip_hdr* s_hdr = (pjsip_hdr*) pjsip_msg_find_hdr_by_name(msg, (const pj_str_t *) &header_name, NULL);
		if (s_hdr) {
			SipHeader SHdr;
			SHdr.fromPj(s_hdr);
			if (check->hdr.hValue == "" || check->hdr.hValue == SHdr.hValue) {
				LOG(logINFO) <<__FUNCTION__<< " header found and value is matching:" << SHdr.hName <<" "<< SHdr.hValue;
				check->result = true;
			} else {
				LOG(logINFO) <<__FUNCTION__<< " header found and value is not matching:" << SHdr.hName <<" "<< SHdr.hValue;
			}
		} else {
			LOG(logINFO) <<__FUNCTION__<< " header not found";
		}
	}
}

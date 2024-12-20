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

		LOG(logINFO) << __FUNCTION__ << " >>> " << line;

		if (std::regex_match(line, re)) {
			LOG(logINFO) << __FUNCTION__ << ": matching ! [" << e << "]";

			return true;
		}
	}
	LOG(logINFO) << __FUNCTION__ << ": not matching ! [" << e << "]";

	return false;
}


void check_checks(vector<ActionCheck> &checks, pjsip_msg* msg, string message) {
	std::string method = pj2Str(msg->line.req.method.name);

	LOG(logINFO) << __FUNCTION__ << ": " + method;

	for (vector<ActionCheck> :: iterator check = checks.begin(); check != checks.end(); ++check) {
		// Message checks
		if (check->type == "message") {
			if (!check->regex.empty()) {
				if (method != check->method) {
					continue;
				}
				if (check_regex(message, check->regex)) {
					check->result = true;
				}
				if (check->fail_on_match) {
					check->result = not check->result;

					LOG(logINFO) << __FUNCTION__ << ": fail_on_match is true, inverting result to " << check->result;
				}
			}
			continue;
		}

		// Header checks
		if (check->type == "header") {
			if (method != check->method) {
				continue;
			}

			if (check->hdr.hName == "") {
				continue;
			}

			// Special check for RURI header. It's a first line of the message
			if (check->hdr.hName == "RURI") {
				std::istringstream tmp_ss(message);
				std::string ruri;
				std::getline(tmp_ss, ruri);
				// Remove CRLF leftovers
				ruri.pop_back();

				if (check->hdr.hValue == ruri) {
					check->result = true;

					LOG(logINFO) << __FUNCTION__ << " Checking RURI ok: [" << ruri << "] == [" << check->hdr.hValue <<"]";
				} else {
					if (check->hdr.hValue.length() >= 6 && check->hdr.hValue.substr(0,6) == "regex/") {
						std::string header_value_regex = check->hdr.hValue.substr(6);

						if (check_regex(ruri, header_value_regex)) {
							check->result = true;

							LOG(logINFO) << __FUNCTION__ << " Checking RURI ok: [" << ruri << "] ~= [" << header_value_regex <<"]";
						}
					} else {
						LOG(logINFO) << __FUNCTION__ << " Checking RURI failed: [" << ruri << "] != [" << check->hdr.hValue <<"]";
					}
				}

				if (check->fail_on_match) {
					check->result = not check->result;

					LOG(logINFO) << __FUNCTION__ << ": fail_on_match is true, inverting result to " << check->result;
				}
				continue;
			}

			LOG(logINFO) << __FUNCTION__ << " check-header:" << check->hdr.hName << " " << check->hdr.hValue;

			pj_str_t header_name;
			header_name.slen = strlen(check->hdr.hName.c_str());
			header_name.ptr = (char*) check->hdr.hName.c_str();

			pjsip_hdr* s_hdr = (pjsip_hdr*) pjsip_msg_find_hdr_by_name(msg, (const pj_str_t *) &header_name, NULL);
			if (s_hdr) {
				SipHeader SHdr;
				SHdr.fromPj(s_hdr);

				if (check->hdr.hValue == "" || check->hdr.hValue == SHdr.hValue) {
					LOG(logINFO) << __FUNCTION__ << " header found and value is matching:" << SHdr.hName << " " << SHdr.hValue;

					check->result = true;
				} else {
					// Check if we have regex in pattern
					if (check->hdr.hValue.length() >= 6 && check->hdr.hValue.substr(0,6) == "regex/") {
						std::string header_value_regex = check->hdr.hValue.substr(6);

						if (check_regex(SHdr.hValue, header_value_regex)) {
							LOG(logINFO) << __FUNCTION__ << " header found and value is matching in regex style: " << SHdr.hName << " " << SHdr.hValue << " =~ " << header_value_regex;

							check->result = true;
						}
					} else {
						LOG(logINFO) << __FUNCTION__ << " header found and value is not matching: " << SHdr.hName << " " << SHdr.hValue << " != " << check->hdr.hValue;
					}
				}

				if (check->fail_on_match) {
					check->result = not check->result;

					LOG(logINFO) << __FUNCTION__ << ": fail_on_match is true, inverting result to " << check->result;
				}

				continue;
			}
			// If header not found, we consider it as a fail anyways
			LOG(logINFO) << __FUNCTION__ << " header not found";
			continue;
		}

		LOG(logWARNING) << __FUNCTION__ << ": unknown check type: " << check->type;
	}
}

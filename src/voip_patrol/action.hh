/*
 * Voip Patrol
 * @author Julien Chavanton 2018
 */

#ifndef VOIP_PATROL_ACTION_H
#define VOIP_PATROL_ACTION_H

#include "voip_patrol.hh"
#include <iostream>

class Config;

class Action {
	public:
			Action(Config *cfg);
			void do_call() {};
			void do_accept() {};
			void do_wait(bool done, int duration=0);
			void do_register() {};
			void set_config(Config *);
			Config* get_config();
	private:
			Config* config;
};

#endif

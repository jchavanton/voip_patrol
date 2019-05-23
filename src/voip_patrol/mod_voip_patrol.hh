
#ifndef MOD_VOIP_PATROL_H
#define MOD_VOIP_PATROL_H

// #include "mod_voip_patrol.h"
#include "voip_patrol.hh"

pj_status_t vp_on_tx_msg(pjsip_tx_data *tdata);

const char *mod_name = "mod_voip_patrol";

pjsip_module mod_voip_patrol = {
	NULL, NULL,                     /* prev, next.              */
	{ (char *)mod_name, 15 },      /* Name.                    */
	-1,                             /* Id                       */
	//PJSIP_MOD_PRIORITY_APPLICATION, /* Priority                 */
	1,
	NULL,                           /* load()                   */
	NULL,                           /* start()                  */
	NULL,                           /* stop()                   */
	NULL,                           /* unload()                 */
	NULL,                           /* on_rx_request()          */
	NULL,                           /* on_rx_response()         */
	vp_on_tx_msg,                   /* on_tx_request()          */
	NULL,                           /* on_tx_response()         */
	NULL,                           /* on_tsx_state()           */
};

#endif

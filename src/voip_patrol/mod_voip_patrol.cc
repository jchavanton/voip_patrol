#include "voip_patrol.hh"

#define THIS_FILE "mod_voip_patrol.cc"

pj_status_t vp_on_tx_msg(pjsip_tx_data *tdata) {
	/* Important note:
	 *  tp_info field is only valid after outgoing messages has passed
	 *  transport layer. So don't try to access tp_info when the module
	 *  has lower priority than transport layer.
	 */

	pjsip_sip_uri *sip_uri = (pjsip_sip_uri*)tdata->msg->line.req.uri;
	LOG(logINFO) <<__FUNCTION__<<":"<< sip_uri->host.ptr <<" " << sip_uri->transport_param.ptr << "\n" ;

	sip_uri->transport_param.ptr = NULL;
	sip_uri->transport_param.slen = 0;
	pj_str_t buff;
	buff.ptr = tdata->buf.start;
	buff.slen = tdata->buf.cur - tdata->buf.start;

	LOG(logINFO) <<__FUNCTION__<<">>>> :"<<buff.slen <<" "<< sip_uri->host.ptr <<" " << sip_uri->transport_param.ptr << "\n" ;

	char *m = tdata->buf.start;
	const char t[12] = ";transport=";
	if (m[0] == 'A') {
		char *ret = strstr(m, t);
		while (ret && ret[0] != 'S' && ret[1] != 'I') {
			ret[0] = ' ';
			ret++;
		}
	}

//    PJ_LOG(3,(THIS_FILE, ">>> TX %d bytes %s to %s %s:%d:\n"
//                         "%.*s\n"
//                         "--end msg--",
//                         (tdata->buf.cur - tdata->buf.start),
//                         pjsip_tx_data_get_info(tdata),
//                         tdata->tp_info.transport->type_name,
//                         tdata->tp_info.dst_name,
//                         tdata->tp_info.dst_port,
//                         (int)(tdata->buf.cur - tdata->buf.start),
//                         tdata->buf.start));

	/* Always return success, otherwise message will not get sent! */
	return PJ_SUCCESS;
}


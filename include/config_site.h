// #define PJ_IOQUEUE_MAX_HANDLES 1024
// #define FD_SETSIZE PJ_IOQUEUE_MAX_HANDLES
//
//
#define PJ_IOQUEUE_MAX_HANDLES      1024
#define FD_SETSIZE_SETABLE      1
#define __FD_SETSIZE            1024

#define PJSIP_MAX_TRANSPORTS        32
#define PJSIP_MAX_RESOLVED_ADDRESSES    32

#define PJSUA_MAX_ACC       512
#define PJSUA_MAX_CALLS     512
#define PJSUA_MAX_PLAYERS   512

// SRTP
#define PJMEDIA_SRTP_HAS_DTLS           1
// Make send of "100 - Trying" explicit
#define PJSUA_DISABLE_AUTO_SEND_100 1

#define PJ_HAS_IPV6 1

#define PJSIP_TCP_KEEP_ALIVE_INTERVAL    90

// TCP transport idle timeout in seconds (default is 33)
// Increase to 1800s (1/2 hour) to keep connections alive during long calls
#define PJSIP_TRANSPORT_IDLE_TIME 1800

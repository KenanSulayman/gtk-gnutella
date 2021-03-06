/*
 * Generated on Sat Apr  5 12:09:15 2014 by enum-msg.pl -- DO NOT EDIT
 *
 * Command: ../../../scripts/enum-msg.pl drop.lst
 */

#ifndef _if_gen_msg_drop_h_
#define _if_gen_msg_drop_h_

/*
 * Enum count: 51
 */
typedef enum {
	MSG_DROP_BAD_SIZE = 0,
	MSG_DROP_TOO_SMALL,
	MSG_DROP_TOO_LARGE,
	MSG_DROP_WAY_TOO_LARGE,
	MSG_DROP_TOO_OLD,
	MSG_DROP_UNKNOWN_TYPE,
	MSG_DROP_UNEXPECTED,
	MSG_DROP_TTL0,
	MSG_DROP_IMPROPER_HOPS_TTL,
	MSG_DROP_MAX_TTL_EXCEEDED,
	MSG_DROP_THROTTLE,
	MSG_DROP_LIMIT,
	MSG_DROP_TRANSIENT,
	MSG_DROP_PONG_UNUSABLE,
	MSG_DROP_HARD_TTL_LIMIT,
	MSG_DROP_MAX_HOP_COUNT,
	MSG_DROP_ROUTE_LOST,
	MSG_DROP_NO_ROUTE,
	MSG_DROP_DUPLICATE,
	MSG_DROP_OOB_PROXY_CONFLICT,
	MSG_DROP_TO_BANNED,
	MSG_DROP_FROM_BANNED,
	MSG_DROP_SHUTDOWN,
	MSG_DROP_FLOW_CONTROL,
	MSG_DROP_QUERY_NO_NUL,
	MSG_DROP_QUERY_TOO_SHORT,
	MSG_DROP_QUERY_OVERHEAD,
	MSG_DROP_BAD_URN,
	MSG_DROP_MALFORMED_SHA1,
	MSG_DROP_MALFORMED_UTF_8,
	MSG_DROP_BAD_RESULT,
	MSG_DROP_BAD_RETURN_ADDRESS,
	MSG_DROP_HOSTILE_IP,
	MSG_DROP_SHUNNED_IP,
	MSG_DROP_MORPHEUS_BOGUS,
	MSG_DROP_SPAM,
	MSG_DROP_EVIL,
	MSG_DROP_MEDIA,
	MSG_DROP_INFLATE_ERROR,
	MSG_DROP_UNKNOWN_HEADER_FLAGS,
	MSG_DROP_OWN_RESULT,
	MSG_DROP_OWN_QUERY,
	MSG_DROP_ANCIENT_QUERY,
	MSG_DROP_BLANK_SERVENT_ID,
	MSG_DROP_GUESS_MISSING_TOKEN,
	MSG_DROP_GUESS_INVALID_TOKEN,
	MSG_DROP_DHT_INVALID_TOKEN,
	MSG_DROP_DHT_TOO_MANY_STORE,
	MSG_DROP_DHT_UNPARSEABLE,
	MSG_DROP_G2_UNEXPECTED,
	MSG_DROP_NETWORK_CROSSING,

	MSG_DROP_REASON_COUNT
} msg_drop_reason_t;

const char *gnet_stats_drop_reason_name(msg_drop_reason_t x);

const char *gnet_stats_drop_reason_to_string(msg_drop_reason_t x);

#endif /* _if_gen_msg_drop_h_ */

/* vi: set ts=4 sw=4 cindent: */

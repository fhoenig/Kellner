/*
* Copyright (c) 2009, Kargo Global Inc. 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Kargo Global Inc. nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Kargo Global Inc. ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Kargo Global Inc. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Author: Florian Hoenig <rian.flo.h@gmail.com>
*/


#ifndef PHP_EVENT_H
#define PHP_EVENT_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

extern zend_module_entry event_module_entry;
#define phpext_event_ptr &event_module_entry

#ifdef PHP_WIN32
#define PHP_EVENT_API __declspec(dllexport)
#else
#define PHP_EVENT_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* refcount macros */
#ifndef Z_ADDREF_P
#define Z_ADDREF_P(pz)                (pz)->refcount++
#endif

#ifndef Z_DELREF_P
#define Z_DELREF_P(pz)                (pz)->refcount--
#endif

#ifndef Z_SET_REFCOUNT_P
#define Z_SET_REFCOUNT_P(pz, rc)      (pz)->refcount = rc
#endif

PHP_MINIT_FUNCTION(event);
PHP_MSHUTDOWN_FUNCTION(event);
PHP_RINIT_FUNCTION(event);
PHP_RSHUTDOWN_FUNCTION(event);
PHP_MINFO_FUNCTION(event);

/*
 * Basic libevent functions
 */
PHP_FUNCTION(event_base_new);
PHP_FUNCTION(event_base_free);
PHP_FUNCTION(event_base_dispatch);
PHP_FUNCTION(event_base_loop);
PHP_FUNCTION(event_base_loopbreak);
PHP_FUNCTION(event_base_loopexit);
PHP_FUNCTION(event_base_set);
PHP_FUNCTION(event_base_priority_init);

PHP_FUNCTION(event_reinit);
PHP_FUNCTION(event_new);
PHP_FUNCTION(event_free);
PHP_FUNCTION(event_set);
PHP_FUNCTION(event_add);
PHP_FUNCTION(event_del);

/*
 * Buffer functions
 */
PHP_FUNCTION(evbuffer_new);
PHP_FUNCTION(evbuffer_free);
PHP_FUNCTION(evbuffer_add);
PHP_FUNCTION(evbuffer_readline);

/*
 * Buffered Events
 */
PHP_FUNCTION(bufferevent_new);
PHP_FUNCTION(bufferevent_enable);
PHP_FUNCTION(bufferevent_disable);
PHP_FUNCTION(bufferevent_read);
PHP_FUNCTION(bufferevent_write);

/*
 * HTTP Client/Server functions
 */
PHP_FUNCTION(evhttp_start);
PHP_FUNCTION(evhttp_set_gencb);
PHP_FUNCTION(evhttp_connection_new);
PHP_FUNCTION(evhttp_connection_set_closecb);
PHP_FUNCTION(evhttp_request_new);
PHP_FUNCTION(evhttp_request_free);
PHP_FUNCTION(evhttp_make_request);

/*
 * HTTP Request functions
 */
PHP_FUNCTION(evhttp_request_get_uri);
PHP_FUNCTION(evhttp_request_method);
PHP_FUNCTION(evhttp_request_body);
PHP_FUNCTION(evhttp_request_input_buffer);
PHP_FUNCTION(evhttp_request_headers);
PHP_FUNCTION(evhttp_request_find_header);
PHP_FUNCTION(evhttp_request_add_header);
PHP_FUNCTION(evhttp_request_status);
PHP_FUNCTION(evhttp_request_append_body);

/*
 * Response functions
 */
PHP_FUNCTION(evhttp_response_set);
PHP_FUNCTION(evhttp_response_add_header);

/*
 * Network byte-order conversion functions
 *
 * Some PHP version have problems unpacking on 64-bit platforms.
 * These functions serve as a workaround for that. Useful for many binary
 * protocol implementations.
 */
PHP_FUNCTION(ntohs);
PHP_FUNCTION(ntohl);
PHP_FUNCTION(htons);
PHP_FUNCTION(htonl);

/*
 *
  	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(event)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(event)
*/

#define PHP_EVENT_BASE_RES_NAME "EVENT BASE"
#define PHP_EVENT_RES_NAME "EVENT"
#define PHP_BUFFEREVENT_RES_NAME "BUFFEREVENT"
#define PHP_EVHTTP_RES_NAME "EVHTTP"
#define PHP_EVHTTP_REQUEST_RES_NAME "EVHTTP Request"
#define PHP_EVHTTP_CONNECTION_RES_NAME "EVHTTP Connection"
#define PHP_EVBUFFER_RES_NAME "EVBUFFER"
#define PHP_EVHTTP_RESPONSE_RES_NAME "EVHTTP Response"


/* In every utility function you add that needs to use variables
   in php_event_globals, call TSRMLS_FETCH(); after declaring other
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as EVENT_G(variable).  You are
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define EVENT_G(v) TSRMG(event_globals_id, zend_event_globals *, v)
#else
#define EVENT_G(v) (event_globals.v)
#endif

/*
 * PHP Event Base struct
 */
typedef struct _php_event_base {
	struct event_base *base;
	int rsrc_id;
	zend_uint events;
} php_event_base;

/*
 * PHP Event struct
 */
typedef struct _php_event
{
	zval *stream;
	zval *cb;
	zval *flags;
	zval *event;
#ifdef ZTS
	TSRMLS_D;
#endif
} php_event;

/*
 * PHP Event struct for bufferevent
 */
typedef struct _php_bufferevent
{
	zval *res_bufferevent;
	zval *stream;
	zval *r_cb;
	zval *w_cb;
	zval *e_cb;
#ifdef ZTS
	TSRMLS_D;
#endif
} php_bufferevent;

/*
 * PHP Event struct for httpevent
 */
typedef struct _php_httpevent
{
	zval *res_httpevent;
	zval *r_cb;
#ifdef ZTS
	TSRMLS_D;
#endif
} php_httpevent;

/*
 * PHP Event struct for evhttp_connection onClose
 */
typedef struct _php_httpcon
{
	zval *res_httpcon;
	zval *c_cb;
#ifdef ZTS
	TSRMLS_D;
#endif
} php_httpcon;

/*
 * Custom event struct that has a http response code and message associated
 * with the buffer. Note that libevent uses the evhttp_request struct for
 * both requests and responses -- this just keeps things cleaner for PHP.
 */
typedef struct _evhttp_response
{
	int res_code;
	char* res_message;
	char* res_body;
	int res_body_len;
#ifdef ZTS
	TSRMLS_D;
#endif
} evhttp_response;

typedef struct _evhttp_callback_arg
{
	zval *arg;
#ifdef ZTS
	TSRMLS_D;
#endif
} evhttp_callback_arg;


/* Resource Macros */
#define FETCH_EVENTBASE(zval, base) \
	ZEND_FETCH_RESOURCE(base, php_event_base *, &zval, -1, PHP_EVENT_BASE_RES_NAME, le_event_base)

#define FETCH_EVENT(zval, ev) \
	ZEND_FETCH_RESOURCE(ev, struct event*, &zval, -1, PHP_EVENT_RES_NAME, le_event)

#define FETCH_BUFFEREVENT(zval, bevent) \
	ZEND_FETCH_RESOURCE(bevent, php_bufferevent_t *, &zval, -1, PHP_EVBUFFER_RES_NAME, le_bufferevent)




#endif	/* PHP_EVENT_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

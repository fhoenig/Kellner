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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_event.h"

#ifndef HAVE_TAILQFOREACH
#include <sys/queue.h>
#endif

/* include libevent header */
#include <event.h>
#include <evhttp.h>

/* network byteorder stuff */
#include <arpa/inet.h>

/* If you declare any globals in php_event.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(event)
*/


/* True global resources - no need for thread safety here */
static int le_evhttp;
static int le_evhttp_request;
static int le_evbuffer;
static int le_event;
static int le_bufferevent;
static int le_evhttp_connection;
static int le_evhttp_response;

static struct event_base *current_base;

/* Forward declarations */
static void event_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void bufferevent_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void evhttp_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void evhttp_request_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void evhttp_connection_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void evhttp_response_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

/* Function argument info decarations */
#ifdef ZEND_ENGINE_2
    ZEND_BEGIN_ARG_INFO(bufferevent_read_byref_arginfo, 0)
        ZEND_ARG_PASS_INFO(0)
        ZEND_ARG_PASS_INFO(1)
        ZEND_ARG_PASS_INFO(0)
    ZEND_END_ARG_INFO()
#else
static unsigned char bufferevent_read_byref_arginfo[] = { 2, BYREF_FORCE, BYREF_NONE };
#endif


/* {{{ event_functions[]
 *
 * Every user visible function must have an entry in event_functions[].
 */
zend_function_entry event_functions[] = {
    PHP_FE(event_init, NULL)
    PHP_FE(event_reinit, NULL)
    PHP_FE(event_new, NULL)
    PHP_FE(event_free, NULL)
    PHP_FE(event_set, NULL)
    PHP_FE(event_add, NULL)
    PHP_FE(event_del, NULL)
    PHP_FE(event_dispatch, NULL)
    PHP_FE(event_loopbreak, NULL)
    PHP_FE(evhttp_start, NULL)
    PHP_FE(evhttp_connection_new, NULL)
    PHP_FE(evhttp_connection_set_closecb, NULL)
    PHP_FE(evhttp_request_new, NULL)
    PHP_FE(evhttp_request_free, NULL)
    PHP_FE(evhttp_make_request, NULL)
	PHP_FE(evhttp_set_gencb, NULL)
    PHP_FE(evhttp_request_get_uri, NULL)
    PHP_FE(evhttp_request_method, NULL)
    PHP_FE(evhttp_request_body, NULL)
    PHP_FE(evhttp_request_append_body, NULL)
	PHP_FE(evhttp_request_input_buffer, NULL)
	PHP_FE(evhttp_request_find_header, NULL)
    PHP_FE(evhttp_request_headers, NULL)
	PHP_FE(evhttp_request_add_header, NULL)
	PHP_FE(evhttp_request_status, NULL)
    PHP_FE(evhttp_response_set, NULL)
	PHP_FE(evhttp_response_add_header, NULL)
	PHP_FE(evbuffer_new, NULL)
    PHP_FE(evbuffer_free, NULL)
    PHP_FE(evbuffer_add, NULL)
	PHP_FE(evbuffer_readline, NULL)
	PHP_FE(bufferevent_new, NULL)
	PHP_FE(bufferevent_enable, NULL)
	PHP_FE(bufferevent_disable, NULL)
	PHP_FE(bufferevent_read, bufferevent_read_byref_arginfo)
	PHP_FE(bufferevent_write, NULL)
	PHP_FE(ntohs, NULL)
	PHP_FE(ntohl, NULL)
	PHP_FE(htons, NULL)
	PHP_FE(htonl, NULL)

	{NULL, NULL, NULL}	/* Must be the last line in event_functions[] */
};
/* }}} */

/* {{{ event_module_entry
 */
zend_module_entry event_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"event",
	event_functions,
	PHP_MINIT(event),
	PHP_MSHUTDOWN(event),
	NULL,		/* Replace with NULL if there's nothing to do at request start */
	NULL,	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(event),
#if ZEND_MODULE_API_NO >= 20010901
	"0.2", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_EVENT
ZEND_GET_MODULE(event)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("event.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_event_globals, event_globals)
    STD_PHP_INI_ENTRY("event.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_event_globals, event_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_event_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_event_init_globals(zend_event_globals *event_globals)
{
	event_globals->global_value = 0;
	event_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(event)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/

	/* resource types */
	le_evhttp = zend_register_list_destructors_ex(evhttp_dtor, NULL, PHP_EVHTTP_RES_NAME, module_number);
    le_evhttp_request = zend_register_list_destructors_ex(evhttp_request_dtor, NULL, PHP_EVHTTP_REQUEST_RES_NAME, module_number);
    le_evhttp_connection = zend_register_list_destructors_ex(evhttp_connection_dtor, NULL, PHP_EVHTTP_CONNECTION_RES_NAME, module_number);
    le_evbuffer = zend_register_list_destructors_ex(NULL, NULL, PHP_EVBUFFER_RES_NAME, module_number);
    le_event = zend_register_list_destructors_ex(event_dtor, NULL, PHP_EVENT_RES_NAME, module_number);
    le_bufferevent = zend_register_list_destructors_ex(bufferevent_dtor, NULL, PHP_BUFFEREVENT_RES_NAME, module_number);
        le_evhttp_response = zend_register_list_destructors_ex(evhttp_response_dtor, NULL, PHP_EVHTTP_RESPONSE_RES_NAME, module_number);

    /* libevent constants */
    REGISTER_LONG_CONSTANT("EV_READ", EV_READ, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("EV_WRITE", EV_WRITE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("EV_TIMEOUT", EV_TIMEOUT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("EV_SIGNAL", EV_SIGNAL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("EV_PERSIST", EV_PERSIST, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("EVBUFFER_EOF", EVBUFFER_EOF, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("EVBUFFER_ERROR", EVBUFFER_ERROR, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("EVHTTP_REQ_GET", EVHTTP_REQ_GET, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("EVHTTP_REQ_POST", EVHTTP_REQ_POST, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("EVHTTP_REQ_HEAD", EVHTTP_REQ_HEAD, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("EVHTTP_REQUEST", EVHTTP_REQUEST, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("EVHTTP_RESPONSE", EVHTTP_RESPONSE, CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(event)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(event)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "event support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

PHP_FUNCTION(event_init)
{
    current_base = event_init();
}

PHP_FUNCTION(event_reinit)
{
    event_reinit(current_base);
}


PHP_FUNCTION(event_dispatch)
{
    event_dispatch();
}

PHP_FUNCTION(event_loopbreak)
{
	if (event_loopbreak() == 0)
	{
		RETURN_TRUE;
	}
	else
	{
		RETURN_FALSE;
	}
}

/*
 * Create an event struct and register it as resource
 */
PHP_FUNCTION(event_new)
{
	struct event *e;

    if (!(e = malloc(sizeof(struct event))))
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not create event resource");
        RETURN_FALSE;
    }
    e->ev_arg = NULL;
    //php_printf("Event created...\n");
    ZEND_REGISTER_RESOURCE(return_value, e, le_event);
}

/*
 * Destructor for bufferevent resource
 */
static void bufferevent_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    struct bufferevent *e = (struct bufferevent*)rsrc->ptr;
    int i;

  	if (e->cbarg != NULL)
   	{
   		zval_dtor(((php_bufferevent*)(e->cbarg))->r_cb);
   		FREE_ZVAL(((php_bufferevent*)(e->cbarg))->r_cb);
   		zval_dtor(((php_bufferevent*)(e->cbarg))->w_cb);
   		FREE_ZVAL(((php_bufferevent*)(e->cbarg))->w_cb);
   		zval_dtor(((php_bufferevent*)(e->cbarg))->e_cb);
   		FREE_ZVAL(((php_bufferevent*)(e->cbarg))->e_cb);
   		zval_dtor(((php_bufferevent*)(e->cbarg))->stream);
   		zend_list_delete( Z_RESVAL_P(((php_event*)(e->cbarg))->stream) );
   		FREE_ZVAL(((php_bufferevent*)(e->cbarg))->stream);
   		FREE_ZVAL(((php_bufferevent*)(e->cbarg))->res_bufferevent);
   		free(e->cbarg);
   	}
    bufferevent_free(e);
}


/*
 * Destructor for evhttp resource
 */
static void evhttp_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    evhttp_free((struct evhttp*)rsrc->ptr);
}

/*
 * Destructor for evhttp_request resource
 */
static void evhttp_request_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
}

/*
 * Destructor for evhttp_connection resource
 */
static void evhttp_connection_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    evhttp_connection_free((struct evhttp_connection*)rsrc->ptr);
}

/*
 * Destructor for our evhttp_response resource
 */
static void evhttp_response_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    evhttp_response *r = (evhttp_response*)rsrc->ptr;
    free(r->res_message);
    if (r->res_body_len > 0)
    {
        free(r->res_body);
    }
    free(r);
}


/*
 * Destructor for event resource
 */
static void event_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    struct event *e = (struct event*)rsrc->ptr;
    int i;

    if (event_initialized(e) == EVLIST_INIT)
    {
    	if (e->ev_arg != NULL)
    	{
    		zval_dtor(((php_event*)(e->ev_arg))->cb);
    		FREE_ZVAL(((php_event*)(e->ev_arg))->cb);
    		FREE_ZVAL(((php_event*)(e->ev_arg))->flags);
    		if (IS_RESOURCE == Z_TYPE_P(((php_event*)(e->ev_arg))->stream))
    			zend_list_delete( Z_RESVAL_P(((php_event*)(e->ev_arg))->stream) );
    		free(e->ev_arg);
    	}
    	event_del(e);
    }
    free(e);
}

/*
 * Generic event callback to call into user land
 */
void php_event_callback_handler(int fd, short ev, void *arg)
{
	php_event *event = (php_event*)arg;
#ifdef ZTS
	TSRMLS_D = event->TSRMLS_C;
#endif
	zval **params[3];
	zval *retval = NULL;
	//zval *z_stream;
	//MAKE_STD_ZVAL(z_stream);
	//ZVAL_RESOURCE(z_stream, php_stream_get_resource_id(event->stream));
 	//zend_list_addref(php_stream_get_resource_id(event->stream));

 	//php_debug_zval_dump(&(event->event));

 	params[0] = &(event->stream);
	params[1] = &(event->flags);
	params[2] = &(event->event);

	/* callback into php */
	if (call_user_function_ex(EG(function_table), NULL, event->cb, &retval, 3, params, 0, NULL TSRMLS_CC) == FAILURE)
	{
		 php_error_docref(NULL TSRMLS_CC, E_ERROR, "Callback failed");
	}

	if (retval)
    	zval_ptr_dtor(&retval);

	//zval_ptr_dtor(params[0]);
	//zval_ptr_dtor(params[2]);
	//zval_ptr_dtor(&z_stream);

	//zend_list_delete(php_stream_get_resource_id(event->stream));
	return;
}


PHP_FUNCTION(event_set)
{
	struct event *e;
    char *callable;
    long event_flags;
    int fd;
    zval *e_res, *z_cb, *z_stream;
    php_stream *stream;
    php_event *event_args;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rzlz", &e_res, &z_stream, &event_flags, &z_cb) == FAILURE)
    {
        RETURN_FALSE;
    }

    /* fetch event struct */
    e = (struct event*) zend_fetch_resource(&e_res TSRMLS_CC, -1, PHP_EVENT_RES_NAME, NULL, 1, le_event);
    if (e == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid event resource");
        RETURN_FALSE;
    }
    /* check signal vs stream */
    if (IS_LONG == Z_TYPE_P(z_stream))
    {
    	// we got a signal event. z_stream contains signal number
    	fd = Z_LVAL_P(z_stream);
    }
    else
    {
	    /* fetch stream */
	    php_stream_from_zval(stream, &z_stream);
	    if (!stream)
	    {
	    	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid stream");
	    	RETURN_FALSE;
	    }
	    if (FAILURE == php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT, (void*)&fd, 1) || fd == -1)
	    {
	    	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incompatible stream");
	    	RETURN_FALSE;
	    }
	    /* set non-blocking and non-buffered to not let it step on our hands here */
	    php_stream_set_option(stream, PHP_STREAM_OPTION_BLOCKING, 0, NULL);
	    stream->flags |= PHP_STREAM_FLAG_NO_BUFFER;
    }

#ifdef ZEND_ENGINE_2
    if (!zend_make_callable(z_cb, &callable TSRMLS_CC))
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid callback: '%s'", callable);
        return;
    }
#endif

    efree(callable);

    /* setup event */
    event_args = (php_event*)malloc(sizeof(php_event));

    event_args->cb = z_cb;
    Z_ADDREF_P(event_args->cb);

    MAKE_STD_ZVAL(event_args->flags);
    ZVAL_LONG(event_args->flags, event_flags);

    event_args->stream = z_stream;
    event_args->event = e_res;

    event_set(e, fd, event_flags, php_event_callback_handler, (void*)event_args);

    RETURN_TRUE;
}

PHP_FUNCTION(event_add)
{
	zval *e_res, *z_sec, *z_usec;
	struct event *e;
	struct timeval timeout;
	int ret;
	int argc;

	argc = ZEND_NUM_ARGS();

	/* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|zz", &e_res, &z_sec, &z_usec) == FAILURE)
    {
        RETURN_FALSE;
    }
    /* fetch event struct */
    e = (struct event*) zend_fetch_resource(&e_res TSRMLS_CC, -1, PHP_EVENT_RES_NAME, NULL, 1, le_event);
    if (e == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid event resource");
        RETURN_FALSE;
    }

    /* add event to scheduler */
    timeout.tv_sec = (argc > 1 && Z_TYPE_P(z_sec) == IS_LONG) ? Z_LVAL_P(z_sec) : 0;
    timeout.tv_usec = (argc > 2 && Z_TYPE_P(z_usec) == IS_LONG) ? Z_LVAL_P(z_usec) : 0;

    if (argc > 1)
    	ret = event_add(e, &timeout);
    else
    	ret = event_add(e, 0);

    /* check if it worked */
    if (ret == 0)
    {
    	RETURN_TRUE;
    }
    else
    {
    	RETURN_FALSE;
    }

}

PHP_FUNCTION(event_del)
{
	zval *e_res;
	struct event *e;

	/* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &e_res) == FAILURE)
    {
        RETURN_FALSE;
    }

    /* fetch event struct */
    e = (struct event*) zend_fetch_resource(&e_res TSRMLS_CC, -1, PHP_EVENT_RES_NAME, NULL, 1, le_event);
    if (e == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid event resource");
        RETURN_FALSE;
    }

    /* remove event from scheduler */
    if (event_del(e) == 0)
    {
    	RETURN_TRUE;
    }
    else
    {
    	RETURN_FALSE;
    }

}

PHP_FUNCTION(event_free)
{
	zval *e_res;

	/* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &e_res) == FAILURE)
    {
        RETURN_FALSE;
    }

    zval_dtor(e_res);

}


PHP_FUNCTION(evhttp_response_set)
{
    long http_code;
	char *content, *http_message;
	int content_len, http_message_len;
	evhttp_response *response;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sls", &content, &content_len, &http_code, &http_message, &http_message_len) == FAILURE)
	{
	    RETURN_FALSE;
	}

	/* TODO: use evbuffer here, instead of passing string around in the struct */
	response = malloc(sizeof(evhttp_response));
	response->res_code = http_code;
	response->res_message = strdup(http_message);
	response->res_body_len = content_len;
	if (content_len > 0)
	{
	    response->res_body = malloc(content_len+1);
	    memcpy(response->res_body, content, content_len);
	}

	ZEND_REGISTER_RESOURCE(return_value, response, le_evhttp_response);
}

void php_callback_handler(struct evhttp_request *req, void *arg)
{
    zval *cb;
    zval *retval = NULL;
    zval **params[1];
    zval *req_resource;
    cb = ((evhttp_callback_arg *)arg)->arg;
    int res;
    struct evbuffer *buf;
    evhttp_response *response;
    char *str;
    int str_len;
    int res_id;
    void *retval_res;
    int retval_res_type;
#ifdef ZTS
	TSRMLS_D = ((evhttp_callback_arg *) arg)->TSRMLS_C;
#endif


    /* pass the request as a php resource */
    MAKE_STD_ZVAL(req_resource);
    res_id = zend_register_resource(req_resource, req, le_evhttp_request);
    params[0] = &req_resource;

    res = call_user_function_ex(EG(function_table), NULL, cb, &retval, 1, params, 0, NULL TSRMLS_CC);

    /* free resource */
    req->cb_arg = NULL;
    zend_list_delete(res_id);
    FREE_ZVAL(req_resource);

    if (!retval)
    {
        /* We got nothing back from user callback */
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Request callback returned nothing");
        buf = evbuffer_new();
        evhttp_send_reply(req, HTTP_SERVUNAVAIL, "ERR", buf);
        return;
    }

    switch (retval->type)
    {
       case IS_RESOURCE:
            /* the callback is allowed to return either a simple evbuffer, or
             * a custom response (incl. response code/message) */
            retval_res = zend_list_find(Z_RESVAL_P(retval), &retval_res_type);
            if (retval_res_type == le_evbuffer)
            {
                buf = (struct evbuffer*) retval_res;
                /* got a evbuffer back. send it! */
                evhttp_send_reply(req, HTTP_OK, "OK", buf);
            } else if (retval_res_type == le_evhttp_response) {
                /* got a response resource back */
                response = (evhttp_response *) retval_res;
                buf = evbuffer_new();
                if (response->res_body_len > 0)
                {
                    evbuffer_add(buf, response->res_body, response->res_body_len);
                }
                evhttp_send_reply(req, response->res_code, response->res_message, buf);
            } else {
                /* We got the wrong resource type */
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Request callback returned illegal resource");
                buf = evbuffer_new();
                evhttp_send_reply(req, HTTP_SERVUNAVAIL, "ERR", buf);
            }
            break;

       case IS_STRING:
           /* return value was a string */
           str = Z_STRVAL_P(retval);
           str_len = Z_STRLEN_P(retval);
           buf = evbuffer_new();
           evbuffer_add(buf, str, str_len);
           evhttp_send_reply(req, HTTP_OK, "OK", buf);
           break;

       default:
           /* We got nothing back from user callback */
           php_error_docref(NULL TSRMLS_CC, E_WARNING, "Request callback returned wrong datatype");
           buf = evbuffer_new();
           evhttp_send_reply(req, HTTP_OK, "OK", buf);
    }

    zval_ptr_dtor(&retval);
    evbuffer_free(buf);
    return;
}

PHP_FUNCTION(evhttp_start)
{
    struct evhttp *httpd;
    char *listen_ip;
    long port;
    int listen_ip_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &listen_ip, &listen_ip_len, &port) == FAILURE)
    {
        RETURN_NULL();
    }

    httpd = evhttp_start(listen_ip, port);

    if (!httpd)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                "Error binding httpd on %s port %d", listen_ip, port);
        RETURN_FALSE;
    }

    /* Set timeout to a reasonably short value for performance [BB-MH] */
    evhttp_set_timeout(httpd, 10);

    ZEND_REGISTER_RESOURCE(return_value, httpd, le_evhttp);
}


PHP_FUNCTION(evhttp_set_gencb)
{
    struct evhttp *httpd;
    zval *res_httpd, *php_cb;
    zval *cb;
    char *callable = NULL;
	evhttp_callback_arg *cb_arg = (evhttp_callback_arg *) emalloc(sizeof(evhttp_callback_arg));

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz", &res_httpd, &php_cb) == FAILURE)
    {
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(httpd, struct evhttp*, &res_httpd, -1, PHP_EVHTTP_RES_NAME, le_evhttp);

#ifdef ZEND_ENGINE_2
    if (!zend_make_callable(php_cb, &callable TSRMLS_CC))
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "function '%s' is not a valid callback", callable);
        efree(callable);
        return;
    }
#endif

    MAKE_STD_ZVAL(cb_arg->arg);
    *(cb_arg->arg) = *php_cb;
    zval_copy_ctor(cb_arg->arg);
#ifdef ZTS
	cb_arg->TSRMLS_C = TSRMLS_C;
#endif

    evhttp_set_gencb(httpd, php_callback_handler, (void *)cb_arg);
}


PHP_FUNCTION(evhttp_request_get_uri)
{
    struct evhttp_request *req;
    zval *res_req;
    zval *z_uri;
    const char *uri;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res_req) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not fetch resource");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);
    uri = evhttp_request_get_uri(req);

    ZVAL_STRING(return_value, (char*)uri, 1);
    return;
}

PHP_FUNCTION(evhttp_request_method)
{
        struct evhttp_request *req;
        zval *res_req;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res_req) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not fetch resource");
        RETURN_FALSE;
    }

        ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);
        /* replicate logic of libevent's internal method (evhttp_method), which isn't exposed in the library */
        switch (req->type) {
                case EVHTTP_REQ_GET:
                        RETURN_STRING("GET", 1);
                case EVHTTP_REQ_POST:
                        RETURN_STRING("POST", 1);
                case EVHTTP_REQ_HEAD:
                        RETURN_STRING("HEAD", 1);
                case EVHTTP_REQ_PUT:
                        RETURN_STRING("PUT", 1);
                case EVHTTP_REQ_DELETE:
                        RETURN_STRING("DELETE", 1);
/*                case EVHTTP_REQ_OPTIONS:
                        RETURN_STRING("OPTIONS", 1);
                case EVHTTP_REQ_TRACE:
                        RETURN_STRING("TRACE", 1);
                case EVHTTP_REQ_CONNECT:
                        RETURN_STRING("CONNECT", 1); */
                default:
                        RETURN_NULL();
        }
}

PHP_FUNCTION(evhttp_request_find_header)
{
    struct evhttp_request *req;
    zval *res_req;
    const char *headervalue;
    char *headername;
    int headername_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &res_req, &headername, &headername_len) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not fetch resource");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);
    if ((headervalue = evhttp_find_header(req->input_headers, headername)) == NULL)
    {
    	/* header not found */
    	RETURN_NULL();
    }

    ZVAL_STRING(return_value, (char*)headervalue, 1);
    return;
}

PHP_FUNCTION(evhttp_request_headers)
{
    struct evhttp_request *req;
    zval *res_req;
    struct evkeyval *header;
	struct evkeyvalq *q;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res_req) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not fetch resource");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);

	array_init(return_value);
	q = req->input_headers;

    TAILQ_FOREACH (header, q, next)
	{
		add_assoc_string(return_value, header->key, header->value, TRUE);
	}
    return;
}

PHP_FUNCTION(evhttp_request_add_header)
{
    struct evhttp_request *req;
    zval *res_req;
    char *headername, *headervalue;
    int headername_len, headervalue_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &res_req, &headername, &headername_len, &headervalue, &headervalue_len) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "error reading parameters");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);
    if (evhttp_add_header(req->input_headers, headername, headervalue) != 0)
    {
    	RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(evhttp_response_add_header)
{
    struct evhttp_request *req;
    zval *res_req;
    char *headername, *headervalue;
    int headername_len, headervalue_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &res_req, &headername, &headername_len, &headervalue, &headervalue_len) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "error reading parameters");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);
	// NOTE: only difference of response headers is below
    if (evhttp_add_header(req->output_headers, headername, headervalue) != 0)
    {
    	RETURN_FALSE;
    }

    RETURN_TRUE;
}


PHP_FUNCTION(evhttp_request_status)
{
    struct evhttp_request *req;
    zval *res_req;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res_req) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "error reading parameters");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);
    RETURN_LONG(req->response_code);
}


PHP_FUNCTION(evhttp_request_input_buffer)
{
    struct evhttp_request *req;
    zval *res_req;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res_req) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not fetch resource");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);
    ZEND_REGISTER_RESOURCE(return_value, req->input_buffer, le_evbuffer);
}

PHP_FUNCTION(evhttp_request_body)
{
    struct evhttp_request *req;
    zval *res_req;
    int body_len, status;
    char *body;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res_req) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not fetch resource");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);

	body_len = evbuffer_get_length(req->input_buffer);

    if (req->input_buffer == NULL || body_len == 0)
    {
    	RETURN_FALSE;
    }

    body = emalloc(body_len + 1);
    status = evbuffer_remove(req->input_buffer, (void *) body, body_len + 1);
	if (status == -1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not get data from resource");
		RETURN_FALSE;
	}

    ZVAL_STRING(return_value, body, 0);
}

PHP_FUNCTION(evhttp_request_append_body)
{
    struct evhttp_request *req;
    zval *res_req;
    int body_len;
    char *body;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &res_req, &body, &body_len) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "error reading params");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(req, struct evhttp_request*, &res_req, -1, PHP_EVHTTP_REQUEST_RES_NAME, le_evhttp_request);

    if (req->output_buffer == NULL)
    {
    	RETURN_FALSE;
    }

    if (evbuffer_add(req->output_buffer, body, body_len) != 0)
    {
    	RETURN_FALSE;
    }
    RETURN_TRUE;
}


PHP_FUNCTION(evbuffer_new)
{
    struct evbuffer *buf;
    buf = evbuffer_new();

    if (buf == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Couldn't create evbuffer.");
        RETURN_FALSE;
    }

    ZEND_REGISTER_RESOURCE(return_value, buf, le_evbuffer);
}

PHP_FUNCTION(evbuffer_free)
{
    struct evbuffer *buf;
    zval *res_buf;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res_buf) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Couldn't free evbuffer");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(buf, struct evbuffer*, &res_buf, -1, PHP_EVBUFFER_RES_NAME, le_evbuffer);
    evbuffer_free(buf);
    RETURN_TRUE;
}

PHP_FUNCTION(evbuffer_add)
{
    struct evbuffer *buf;
    zval *res_buf;
    char *str;
    int str_len;
    int added;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &res_buf, &str, &str_len) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "evbuffer resource and string required");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(buf, struct evbuffer*, &res_buf, -1, PHP_EVBUFFER_RES_NAME, le_evbuffer);
    added = evbuffer_add(buf, str, str_len);
    RETVAL_LONG(added);
    return;
}

PHP_FUNCTION(evbuffer_readline)
{
    struct evbuffer *buf;
    zval *res_buf;
    char *line;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res_buf) == FAILURE)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "evbuffer resource required");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE(buf, struct evbuffer*, &res_buf, -1, PHP_EVBUFFER_RES_NAME, le_evbuffer);
    line = evbuffer_readline(buf);
	if (line == 0)
	{
		RETURN_FALSE;
	}
	else
	{
		ZVAL_STRING(return_value, line, 1);
	}
}


/**
 * Called by libevent when there is data to read.
 */
void callback_buffered_on_read(struct bufferevent *bev, void *arg)
{
	php_bufferevent *event = (php_bufferevent*)arg;
#ifdef ZTS
	TSRMLS_D = event->TSRMLS_C;
#endif
	zval **params[1];
	zval *retval = NULL;

	params[0] = &(event->res_bufferevent);

	/* callback into php */
	if (call_user_function_ex(EG(function_table), NULL, event->r_cb, &retval, 1, params, 0, NULL TSRMLS_CC) == FAILURE)
	{
		 php_error_docref(NULL TSRMLS_CC, E_ERROR, "Callback failed");
	}

	if (retval)
    	zval_ptr_dtor(&retval);

	return;
}

/**
 * Called by libevent when the write buffer reaches 0. We only
 * provide this because libevent expects it, but we don't use it.
 */
void callback_buffered_on_write(struct bufferevent *bev, void *arg)
{
	php_bufferevent *event = (php_bufferevent*)arg;
#ifdef ZTS
	TSRMLS_D = event->TSRMLS_C;
#endif
	zval **params[1];
	zval *retval = NULL;

	params[0] = &(event->res_bufferevent);

	/* callback into php */
	if (call_user_function_ex(EG(function_table), NULL, event->w_cb, &retval, 1, params, 0, NULL TSRMLS_CC) == FAILURE)
	{
		 php_error_docref(NULL TSRMLS_CC, E_ERROR, "Callback failed");
	}

	if (retval)
    	zval_ptr_dtor(&retval);

	return;
}

/**
 * Called by libevent when there is an error on the underlying socket
 * descriptor.
 */
void callback_buffered_on_error(struct bufferevent *bev, short what, void *arg)
{
	php_bufferevent *event = (php_bufferevent*)arg;
#ifdef ZTS
	TSRMLS_D = event->TSRMLS_C;
#endif
	zval **params[3];
	zval *code;
	zval *retval = NULL;

 	params[0] = &(event->stream);
	params[1] = &(event->res_bufferevent);

	MAKE_STD_ZVAL(code);
	ZVAL_LONG(code, what);
	params[2] = &code;

	/* callback into php */
	if (call_user_function_ex(EG(function_table), NULL, event->e_cb, &retval, 3, params, 0, NULL TSRMLS_CC) == FAILURE)
	{
		 php_error_docref(NULL TSRMLS_CC, E_ERROR, "Callback failed");
	}

	if (retval)
    	zval_ptr_dtor(&retval);

	zval_ptr_dtor(&code);

	return;
}


PHP_FUNCTION(bufferevent_new)
{
	struct bufferevent *e;
	char *r_callable, *w_callable, *e_callable;
    int fd;
    zval *z_rcb, *z_wcb, *z_ecb, *z_stream;
    php_stream *stream;
    php_bufferevent *be;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzzz", &z_stream, &z_rcb, &z_wcb, &z_ecb) == FAILURE)
    {
        RETURN_FALSE;
    }

    /* fetch stream */
    php_stream_from_zval(stream, &z_stream);
    if (!stream)
    {
    	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid stream");
    	RETURN_FALSE;
    }
    if (FAILURE == php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT, (void*)&fd, 1) || fd == -1)
    {
    	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Incompatible stream");
    	RETURN_FALSE;
    }

    /* set non-blocking and non-buffered to not let it step on our hands here */
    php_stream_set_option(stream, PHP_STREAM_OPTION_BLOCKING, 0, NULL);
    stream->flags |= PHP_STREAM_FLAG_NO_BUFFER;


    /* make callbacks callable */
#ifdef ZEND_ENGINE_2
    if (!zend_make_callable(z_rcb, &r_callable TSRMLS_CC))
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid callback: '%s'", r_callable);
        return;
    }
    if (!zend_make_callable(z_wcb, &w_callable TSRMLS_CC))
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid callback: '%s'", w_callable);
        return;
    }
    if (!zend_make_callable(z_ecb, &e_callable TSRMLS_CC))
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid callback: '%s'", e_callable);
        return;
    }
#endif

    efree(r_callable);
    efree(w_callable);
    efree(e_callable);

    /* allocate bufferevent struct for extension use */
    if (!(be = malloc(sizeof(php_bufferevent))))
    {
     	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not create php_bufferevent internal struct");
       	RETURN_FALSE;
    }

    /* make struct with all callback functions, stream and the event resource */
    be->r_cb = z_rcb;
    be->w_cb = z_wcb;
    be->e_cb = z_ecb;
    Z_ADDREF_P(be->r_cb);
    Z_ADDREF_P(be->w_cb);
    Z_ADDREF_P(be->e_cb);

    /* call libevent */
    if (NULL == (e = bufferevent_new(fd, callback_buffered_on_read, callback_buffered_on_write, callback_buffered_on_error, be)))
    {
        Z_DELREF_P(be->r_cb);
        Z_DELREF_P(be->w_cb);
        Z_DELREF_P(be->e_cb);
    	free(be);
    	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not create bufferevent");
    	RETURN_FALSE;
    }

    be->stream = z_stream;
    Z_ADDREF_P(be->stream);

    MAKE_STD_ZVAL(be->res_bufferevent);
    ZEND_REGISTER_RESOURCE(be->res_bufferevent, e, le_bufferevent);
    RETVAL_ZVAL(be->res_bufferevent, 0, 0);
}

PHP_FUNCTION(bufferevent_enable)
{
	zval *e_res;
	struct bufferevent *e;
	long flags;

	/* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &e_res, &flags) == FAILURE)
    {
        RETURN_FALSE;
    }

    /* fetch event struct */
    e = (struct bufferevent*) zend_fetch_resource(&e_res TSRMLS_CC, -1, PHP_BUFFEREVENT_RES_NAME, NULL, 1, le_bufferevent);
    if (e == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid bufferevent resource");
        RETURN_FALSE;
    }

    /* enable event in the scheduler */
    if (bufferevent_enable(e, flags) == 0)
    {
    	RETURN_TRUE;
    }
    else
    {
    	RETURN_FALSE;
    }

}

PHP_FUNCTION(bufferevent_disable)
{
	zval *e_res;
	struct bufferevent *e;
	long flags;

	/* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &e_res, &flags) == FAILURE)
    {
        RETURN_FALSE;
    }

    /* fetch event struct */
    e = (struct bufferevent*) zend_fetch_resource(&e_res TSRMLS_CC, -1, PHP_BUFFEREVENT_RES_NAME, NULL, 1, le_bufferevent);
    if (e == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid bufferevent resource");
        RETURN_FALSE;
    }

    /* disable event in the scheduler */
    if (bufferevent_disable(e, flags) == 0)
    {
    	RETURN_TRUE;
    }
    else
    {
    	RETURN_FALSE;
    }
}


PHP_FUNCTION(bufferevent_read)
{
	zval *target, *e_res;
	struct bufferevent *e;
    long count;
    int bytesread;

	/* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzl", &e_res, &target, &count) == FAILURE)
    {
        RETURN_FALSE;
    }

    if (!PZVAL_IS_REF(target) || count < 1)
    {
        RETURN_FALSE;
    }

    /* fetch event struct */
    e = (struct bufferevent*) zend_fetch_resource(&e_res TSRMLS_CC, -1, PHP_BUFFEREVENT_RES_NAME, NULL, 1, le_bufferevent);
    if (e == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid bufferevent resource");
        RETURN_FALSE;
    }

    convert_to_string(target);

    /* enlarge */
    Z_STRVAL_P(target) = erealloc(Z_STRVAL_P(target), Z_STRLEN_P(target) + count + 1);
    bytesread = bufferevent_read(e, Z_STRVAL_P(target) + Z_STRLEN_P(target), count + 1);

    if (bytesread < count)
    {
    	// TODO: try to avoid realloc if less data than count was available
    	Z_STRVAL_P(target) = erealloc(Z_STRVAL_P(target), Z_STRLEN_P(target) + bytesread + 1);
    	count = bytesread;
    }

    Z_STRLEN_P(target) += count;
    *(Z_STRVAL_P(target) + Z_STRLEN_P(target)) = 0;
    RETURN_LONG(count);
}


PHP_FUNCTION(bufferevent_write)
{
	zval *e_res;
	struct bufferevent *e;
	char *data;
	int count;

	/* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zs", &e_res, &data, &count) == FAILURE)
    {
        RETURN_FALSE;
    }

    /* fetch event struct */
    e = (struct bufferevent*) zend_fetch_resource(&e_res TSRMLS_CC, -1, PHP_BUFFEREVENT_RES_NAME, NULL, 1, le_bufferevent);
    if (e == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid bufferevent resource");
        RETURN_FALSE;
    }

    /* write */
    if (bufferevent_write(e, data, count) != 0)
    {
    	RETURN_FALSE;
    }

    RETURN_TRUE;
}


PHP_FUNCTION(ntohs)
{
	char *in;
	int in_len;

	if ((zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &in, &in_len) == FAILURE) || (in_len != 2))
	{
	    php_error_docref(NULL TSRMLS_CC, E_WARNING, "parameter expected to be a 2-byte string in network byte order");
	    RETURN_FALSE;
	}

	RETVAL_DOUBLE(ntohs(*((uint16_t*)in)));
	return;
}


PHP_FUNCTION(ntohl)
{
	char *in;
	int in_len;


	if ((zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &in, &in_len) == FAILURE) || (in_len != 4))
	{
	    php_error_docref(NULL TSRMLS_CC, E_WARNING, "parameter expected to be a 4-byte string in network byte order");
	    RETURN_FALSE;
	}

	RETVAL_DOUBLE(ntohl(*((uint32_t*)in)));
	return;
}


PHP_FUNCTION(htons)
{
	char *in;
	int in_len;

	if ((zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &in, &in_len) == FAILURE) || (in_len != 2))
	{
	    php_error_docref(NULL TSRMLS_CC, E_WARNING, "parameter expected to be a 4-byte string in host byte order");
	    RETURN_FALSE;
	}

	RETVAL_DOUBLE(htons(*((uint16_t*)in)));
	return;

}


PHP_FUNCTION(htonl)
{
	char *in;
	int in_len;

	if ((zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &in, &in_len) == FAILURE) || (in_len != 4))
	{
	    php_error_docref(NULL TSRMLS_CC, E_WARNING, "parameter expected to be a 4-byte string in host byte order");
	    RETURN_FALSE;
	}

	RETVAL_DOUBLE(htonl(*((uint32_t*)in)));
	return;
}


PHP_FUNCTION(evhttp_connection_new)
{
	struct evhttp_connection *con;
    char *host_ip;
    long port;
    int host_ip_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &host_ip, &host_ip_len, &port) == FAILURE)
    {
        RETURN_NULL();
    }

    con = evhttp_connection_new(host_ip, port);


    if (!con)
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                "Error connection to %s port %d", host_ip, port);
        RETURN_FALSE;
    }

    ZEND_REGISTER_RESOURCE(return_value, con, le_evhttp_connection);
}

/**
 * Called by libevent when http connection closes
 */
void callback_connection_on_close(struct evhttp_connection *con, void *arg)
{
	php_httpcon *connection = (php_httpcon*)arg;
#ifdef ZTS
	TSRMLS_D = connection->TSRMLS_C;
#endif
	zval **params[1];
	zval *retval = NULL;

	params[0] = &(connection->res_httpcon);

	/* callback into php */
	if (call_user_function_ex(EG(function_table), NULL, connection->c_cb, &retval, 1, params, 0, NULL TSRMLS_CC) == FAILURE)
	{
		 php_error_docref(NULL TSRMLS_CC, E_ERROR, "Callback failed");
	}

	if (retval)
    	zval_ptr_dtor(&retval);

	/* free internal arg struct */
	zval_dtor(connection->c_cb);
	FREE_ZVAL(connection->c_cb);
	FREE_ZVAL(connection->res_httpcon);
	free(connection);

	return;
}


PHP_FUNCTION(evhttp_connection_set_closecb)
{
	struct evhttp_connection *c;
	php_httpcon *hc;
	char *c_callable;
    zval *z_ccb, *con_res;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz", &con_res, &z_ccb) == FAILURE)
    {
        RETURN_FALSE;
    }

    /* fetch connection struct */
    c = (struct evhttp_connection*) zend_fetch_resource(&con_res TSRMLS_CC, -1, PHP_EVHTTP_CONNECTION_RES_NAME, NULL, 1, le_evhttp_connection);
    if (c == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid evhttp_connection resource");
        RETURN_FALSE;
    }

    /* make callbacks callable */
#ifdef ZEND_ENGINE_2
    if (!zend_make_callable(z_ccb, &c_callable TSRMLS_CC))
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid callback: '%s'", c_callable);
        return;
    }
#endif

    efree(c_callable);

    /* allocate httpevent struct for extension use */
    if (!(hc = malloc(sizeof(php_httpcon))))
    {
     	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not create php_httpcon internal struct");
       	RETURN_FALSE;
    }

    /* make struct with all callback functions, stream and the event resource */
    hc->c_cb = z_ccb;
    Z_ADDREF_P(hc->c_cb);
    hc->res_httpcon = con_res;
    Z_ADDREF_P(hc->res_httpcon);

    /* call libevent */
    evhttp_connection_set_closecb(c, callback_connection_on_close, hc);

    RETURN_TRUE;
}


/**
 * Called by libevent when http request is done
 */
void callback_request_on_complete(struct evhttp_request *req, void *arg)
{
	php_httpevent *event = (php_httpevent*)arg;
#ifdef ZTS
	TSRMLS_D = event->TSRMLS_C;
#endif
	zval **params[1];
	zval *retval = NULL;

	params[0] = &(event->res_httpevent);

	/* callback into php */
	if (call_user_function_ex(EG(function_table), NULL, event->r_cb, &retval, 1, params, 0, NULL TSRMLS_CC) == FAILURE)
	{
		 php_error_docref(NULL TSRMLS_CC, E_ERROR, "Callback failed");
	}

	if (retval)
    	zval_ptr_dtor(&retval);

	return;
}

PHP_FUNCTION(evhttp_request_new)
{
	struct evhttp_request *r;
	php_httpevent *he;
	char *r_callable;
    zval *z_rcb;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_rcb) == FAILURE)
    {
        RETURN_FALSE;
    }

    /* make callbacks callable */
#ifdef ZEND_ENGINE_2
    if (!zend_make_callable(z_rcb, &r_callable TSRMLS_CC))
    {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid callback: '%s'", r_callable);
        return;
    }
#endif

    efree(r_callable);

    /* allocate httpevent struct for extension use */
    if (!(he = malloc(sizeof(php_httpevent))))
    {
     	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not create php_httpevent internal struct");
       	RETURN_FALSE;
    }

    /* make struct with all callback functions, stream and the event resource */
    he->r_cb = z_rcb;
    Z_ADDREF_P(he->r_cb);

    /* call libevent */
    if (NULL == (r = evhttp_request_new(callback_request_on_complete, he)))
    {
        Z_DELREF_P(he->r_cb);
    	free(he);
    	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not create httpevent");
    	RETURN_FALSE;
    }

    MAKE_STD_ZVAL(he->res_httpevent);
    ZEND_REGISTER_RESOURCE(he->res_httpevent, r, le_evhttp_request);
    RETVAL_ZVAL(he->res_httpevent, 0, 0);
}


PHP_FUNCTION(evhttp_make_request)
{
	zval *con_res, *req_res;
	struct evhttp_connection *con;
	struct evhttp_request *req;
    long type;
	int url_len;
	char *url;
	int ret;

	/* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rrls", &con_res, &req_res, &type, &url, &url_len) == FAILURE)
    {
        RETURN_FALSE;
    }

    /* fetch connection struct */
    con = (struct evhttp_connection*) zend_fetch_resource(&con_res TSRMLS_CC, -1, PHP_EVHTTP_CONNECTION_RES_NAME, NULL, 1, le_evhttp_connection);
    if (con == NULL)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid evhttp_connection resource");
        RETURN_FALSE;
    }

	/* fetch request struct */
    req = (struct evhttp_request*) zend_fetch_resource(&req_res TSRMLS_CC, -1, PHP_EVHTTP_REQUEST_RES_NAME, NULL, 1, le_evhttp_request);
    if (req == NULL)
    {
    	php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid evhttp_request resource");
    	RETURN_FALSE;
    }

	zend_list_addref(Z_RESVAL_P(req_res));
	//Z_ADDREF_P(req_res);

    ret = evhttp_make_request(con, req, type, url);
	//php_printf("%d, %s, %d\n", type, url, ret);

    RETURN_LONG(ret);
}


PHP_FUNCTION(evhttp_request_free)
{
	struct evhttp_request *req;
	//php_httpevent *he;
	//char *r_callable;
    zval *res_req;

    /* parse parameters */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &res_req) == FAILURE)
    {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid evhttp_request resource");
        RETURN_FALSE;
    }

	/* fetch request struct */
    req = (struct evhttp_request*) zend_fetch_resource(&res_req TSRMLS_CC, -1, PHP_EVHTTP_REQUEST_RES_NAME, NULL, 1, le_evhttp_request);
    if (req == NULL)
    {
    	php_error_docref(NULL TSRMLS_CC, E_ERROR, "First argument must be a valid evhttp_request resource");
    	RETURN_FALSE;
    }

	zend_hash_index_del(&EG(regular_list), Z_RESVAL_P(res_req));
	//evhttp_request_free(req);
	RETURN_TRUE;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

<?php
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

function test_callback($evhttp_request)
{
    echo "script-land memory: ".memory_get_usage()." and real memory from system: ".memory_get_usage(true)."\n";

    $uri = evhttp_request_get_uri($evhttp_request);

    $ct = evhttp_request_find_header($evhttp_request, 'Content-Type');
    echo "URI: $uri\n";
    echo "Content-Type: $ct\n";

    print_r(evhttp_request_headers($evhttp_request));
    echo evhttp_request_body($evhttp_request);
	evhttp_response_add_header($evhttp_request, "Content-Type", "text/plain");

 	return evhttp_response_set("Hello World!", 200, "OK");
}


if(!extension_loaded('event')) {
	dl('event.' . PHP_SHLIB_SUFFIX);
}

event_init();

$httpd = evhttp_start("0.0.0.0", 8080);

evhttp_set_gencb($httpd, 'test_callback');

event_dispatch();



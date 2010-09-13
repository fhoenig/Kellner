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
* Author: Sam Sandberg <sam@kargo.com>
*/


function on_read($stream, $flags, $event)
{
	echo "system: on_read... event=" . print_r($event, true) . "\n";

	$input = fread($stream, 256);
	echo $input;

	if (strpos($input, "username: ") !== false)
	{
		fwrite($stream, "TomBot");
	}
	else if (strpos($input, "cheese") !== false)
	{
		fwrite($stream, "I LOVE CHEESE");
	}
		
	/*//$read = "";
	bufferevent_read($event, $read, 256);
	echo "read some stuff from the socket: $read \n";

	if (strpos($read, "username") !== false)
	{	
		bufferevent_write($event, "TomBot\n");
	}
	else if (strpos($read, "cheese") !== false)
	{
		bufferevent_write($event, "I LOVE CHEESE\n");
	}*/
}

function on_write($stream, $flags, $event)
{
	#echo "system: on_write...\n";
	//fwrite($stream, "joe\n");
	//fwrite($stream, "from fwrite()\n");

	//echo "in on_write\n";
	//bufferevent_write($event, "nickname\n");
	//bufferevent_write($event, "message\n");
}


function on_timer($empty, $flags, $event)
{
	echo "system: on_timer...\n";
    echo "RING!!! RING!!!\n";
    //event_add($event, 0, 500000);
}

if (!extension_loaded('event'))
{
	dl('event.' . PHP_SHLIB_SUFFIX);
}

echo "system: extension loaded - calling event_init()\n";
event_init();

/* setup php stream */
$errno = 0;
$errstr = "";

$socket = stream_socket_client("tcp://192.168.2.116:8000", $errno, $errstr, 0, STREAM_CLIENT_ASYNC_CONNECT);
echo "system: socket connected\n";

if (!$socket)
{
	echo "$errstr ($errno)\n";
	exit;
}

echo "system: setting up on_read\n";
$read_event = event_new();
event_set($read_event, $socket, EV_READ | EV_PERSIST , 'on_read');
event_add($read_event);

echo "system: setting up on_write\n";
$write_event = event_new();
event_set($write_event, $socket, EV_WRITE | EV_PERSIST, 'on_write');
event_add($write_event);

echo "system: setting up on_timer\n";
$timer_event = event_new();
event_set($timer_event, -1, 0, 'on_timer');
event_add($timer_event, 15, 0);

/* go! */
echo "system: dispatching...\n";
event_dispatch();

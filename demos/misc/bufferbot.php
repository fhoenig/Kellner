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


$CLIENTS = array();

function on_readable($event)
{
//	$read = "";
	print "in on_readable with event = " . print_r($event, true) . "\n";
	global $event;
	bufferevent_read($event, $read, 100);
	bufferevent_write($event, "test");

/*
	if(strpos($read, "username") !== false)	
		bufferevent_write($event, "TomBot\n");
	else if(strpos($read, "cheese") !== false)
		bufferevent_write($event, "I LOVE CHEESE\n");
	*/
}

function on_writeable($event){
	
	bufferevent_write($event, 'in on_writeable');
	
	
}


function on_read($event)
{
	$read = "on_read: ";
	bufferevent_read($event, $read, 100);
	bufferevent_write($event, $read);
}


/*
function on_error($stream, $event, $code)
{
	if ($reason | EVBUFFER_EOF)
		echo "client disconnected (EOF).\n";
	else if ($reason | EVBUFFER_ERROR)
		echo "client socket failed.\n";
	else
		echo "client socket died. no idea what happend.\n";
}

function on_write($event)
{
    echo "on_write\n";
}

function on_sigint($signal, $flags, $event)
{
    global $CLIENTS;
    echo "on_sigint: You want me to stop, eh!?\n";
    echo "disconnecting clients...\n";
    foreach ($CLIENTS as $name => $d)
    {
        echo "bye bye $name...";
//        fclose($d[1]);
        echo "OK\n";
    }
    sleep(2);
    exit;
}

function on_accept($stream, $flags, $event)
{
	global $CLIENTS;
	$mem = memory_get_usage();
    echo "on_accept($stream, $flags): mem=$mem \n";
    if (false === ($con = stream_socket_accept($stream, 0, $name)))
    	echo "Accept failed\n";
    else
    {
    	echo "Peer:$name\n";
    	
    	$client = bufferevent_new($con, 'on_read', 'on_write', 'on_error');
    	debug_zval_dump($client);
        // TODO: If bufferevent falls out of scope: disable and free buffer. also close stream.
        $CLIENTS[$name] = array(&$client, &$con);
        
        bufferevent_enable($client, EV_READ);
        
    }

}


function on_writeable($stream, $flags, $event){
	
	
	
}

function on_timer($empty, $flags, $event)
{
    echo "RING!!! RING!!!\n";
    event_add($event, 0, 500000);
}
*/

if(!extension_loaded('event'))
{
	dl('event.' . PHP_SHLIB_SUFFIX);
}

event_init();

/* setup php stream */

$errno = 0;
$errstr = "";


//$socket = stream_socket_client("tcp://192.168.2.116:8000", $errno, $errstr, 2000000, STREAM_CLIENT_PERSISTENT);
$socket = stream_socket_client("tcp://192.168.2.116:8000", $errno, $errstr);
//stream_set_blocking($socket, 0);


if (!$socket)
{
	echo "$errstr ($errno)\n";
	exit;
}


$event = event_new();
event_set($event, $socket, EV_WRITE | EV_PERSIST, 'on_writeable' );
event_add($event);
unset($event);

$event = event_new();
event_set($event, $socket, EV_READ | EV_PERSIST , 'on_read' );
event_add($event);


//$sigint = event_new();
//event_set($sigint, SIGINT, EV_SIGNAL | EV_PERSIST, 'on_sigint');
//event_add($sigint);

//$timer =event_new();
//event_set($timer, -1, 0, 'on_timer');
//event_add($timer, 15, 0);

/* go! */
event_dispatch();


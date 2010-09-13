<?php

function on_complete($req)
{
    echo "request complete: ". evhttp_request_status($req) . "\n";
    //echo evhttp_request_body($req)."\n";
	evhttp_request_free($req);
	echo memory_get_usage()."\n\n";
}

function on_close($con)
{
    echo "connection closed\n";
}


if (!extension_loaded('event'))
{
	dl('event.' . PHP_SHLIB_SUFFIX);
}

event_init();

for ($i=0; $i<200; $i++)
{
	$con = evhttp_connection_new("www.google.com", 80);
	evhttp_connection_set_closecb($con, 'on_close');
	
	$req = evhttp_request_new('on_complete');

	evhttp_request_add_header($req, 'Host', 'www.google.com');
	evhttp_request_add_header($req, 'Connection', 'Keep-Alive');
//	evhttp_request_add_header($req, 'Content-Type', 'text/xml');
/*	$result = evhttp_request_append_body($req, '<?xml foo?><id>'.$i.'</id>');*/
	
	evhttp_make_request($con, $req, EVHTTP_REQ_GET, '/');
}

event_dispatch();

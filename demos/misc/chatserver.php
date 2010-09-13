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



$CLIENTS = array();

$COMMANDS = array(
	'away' => "AWAY [<message>]\nThis command provides the server with a message to automatically send in reply to a PRIVMSG directed at the user, but not to a channel they are on. If <message> is omitted, the away status is removed.",
	'help' => "HELP [<command>]\nThis command will display the help information about a specific command, or a list of all of the commands if no command is specified.",
	'ison' => "ISON <nicknames>\nThis command queries the server to see if the clients in the space-separated list <nicknames> are currently on the network. The server returns only the nicknames that are on the network in a space-separated list. If none of the clients are on the network the server returns an empty list.",
	'join' => "JOIN <channels> [<keys>]\nTODO: This command makes the client join the channels in the comma-separated list <channels>, specifying the passwords, if needed, in the comma-separated list <keys>. If the channel(s) do not exist then they will be created.",
	'kick' => "KICK <client> [<message>] | KICK <channel> <client> [<message>]\nThis command forcibly removes <client> from <channel>. This command may only be issued by channel operators.",
	'kill' => "KILL <client> <comment>\nThis command forcibly removes <client> from the network. This command may only be issued by IRC operators.",
	'list' => "LIST [<channels> [<server>]]\nTODO: This command lists all channels on the server. If the comma-separated list <channels> is given, it will return the channel topics. If <server> is given, the command will be forwarded to <server> for evaluation.",
	'motd' => "MOTD [<server>]\nThis command returns the message of the day on <server> or the current server if it is omitted.",
	'names' => "NAMES [<channels>] | NAMES [<channels> [<server>]]\nThis command returns a list of who is on the comma-separated list of <channels>, by channel name. If <channels> is omitted, all users are shown, grouped by channel name with all users who are not on a channel being shown as part of channel \"*\". If <server> is specified, the command is sent to <server> for evaluation.",
	'nick' => "NICK <nickname>\nThis command allows a client to change their IRC nickname.",
	'privmsg' => "PRIVMSG <msgtarget> <message>\nThis command sends <message> to <msgtarget>, which is usually a user or channel.",
	'quit' => "QUIT [<message>]\nThis command disconnects the user from the server.",
	'time' => "TIME [<server>]\nThis command returns the local time on the current server, or <server> if specified.",
	'users' => "USERS [<server>]\nThis command returns a list of users and information about those users in a format similar to the UNIX commands who, rusers and finger.",
);

$count = 0;

function findClientByResource($resource)
{
	global $CLIENTS;
	foreach($CLIENTS as $ip => $d)
	{
		if ($d['client'] == $resource)
			return $ip;
	}
	return false;
}

function findClientByNick($nick)
{
	global $CLIENTS;
	foreach($CLIENTS as $ip => $d)
	{
		if ($d['nick'] == $nick)
			return $ip;
	}
	return false;
}

function on_read($event)
{
	global $CLIENTS;
	global $COMMANDS;
	
	$time = date('H:i:s');
	bufferevent_read($event, $msg, 100);
	$msg = rtrim($msg);
	
	// determine sender
	$sender = '';
	if(($ip = findClientByResource($event)) == false)
	{
		echo "somethin fucked up - couldn't find sender";
		return;
	}
	
	$client = $CLIENTS[$ip];
	$sender = $client['nick'];
	
	echo $time . ' ' . $sender . ': ' . $msg . "\n";
	
	$toUser = $toOthers = null;
	if (substr(trim($msg), 0, 1) == '/')
	{
		$msg = trim($msg);
		$params = explode(' ', $msg);
		$cmd = strtolower(substr($params[0], 1));
		unset($params[0]);
		$paramStr = trim(implode(' ', $params));
		switch($cmd)
		{
			case 'away':
				$toUser = "This command coming soon, when private messaging is available.";
				break;
			case 'help':
				$paramStr = strtolower($paramStr);
				if (strlen($paramStr) == 0)
					$toUser = "Here is a list of available commands: " . implode(', ', array_keys($COMMANDS));
				else if (in_array($paramStr, array_keys($COMMANDS)))
					$toUser = $COMMANDS[$paramStr];
				else
					$toUser = "No such command as '$paramStr'. Type 'help' for a list of available commands.";
				break;
			case 'ison':
				$toUser = "From the users you listed, the following are online: ";
				foreach($params as $user)
				{
					$user = trim($user);
					if (findClientByNick($user) != false)
						$toUser .= "$user ";
				}
				break;
			case 'join':
				$toUser = "This command coming soon, when there is more than one channel available to join.";
				break;
			case 'kick':
			case 'kill':
				if (($ip = findClientByNick($paramStr)) != false)
				{
    				$toUser = $toOthers = $client['nick'] . " has kicked " . $CLIENTS[$ip]['nick'] . ".";
                    fclose($CLIENTS[$ip]['con']);
                    unset($CLIENTS[$ip]);
				}
				break;
			case 'list':
				$toUser = "This command coming soon, when there is more than one channel available to join.";
				break;
			case 'motd':
				$toUser = "Message Of The Day: Eat a dick.";
				break;
			case 'names':
			case 'users':
				$toUser = "List of people online now: ";
				foreach ($CLIENTS as $client)
					$toUser .= $client['nick'] . ' ';
				break;
			case 'nick':
				$old = $client['nick'];
				$new = trim($paramStr);
				
        		// TODO: maybe use some sort of Zend_Validator to handle this
        		// TODO: make sure username is unique
        		if (strlen($new) == 0)
        		{
					$toUser = "Please specify a valid nick.";
        		}
        		else
        		{
					$CLIENTS[$ip]['nick'] = $new;
					
					$toUser = "You have changed your nick from '$old' to '$new'.";
					$toOthers = "$old has changed their nick to '$new'.";
				}
				break;
			case 'privmsg':
				$toUser = "This command coming soon.";
				break;
			case 'quit':
			case 'exit':
				$toOthers = $client['nick'] . " has disconnected.";
                fclose($client['con']);
                unset($CLIENTS[$ip]);
				break;
			case 'time':
				$toUser = "The current server time is " . date('H:i:s') . ".";
				break;
			default:
				$toUser = "No such command as '$cmd'.";
				break;
		}
	}
	else
	{
		$toUser = $toOthers = "$sender: " . $msg;
	}
	
	// send message to all other clients
	foreach($CLIENTS as $ip => $d)
	{
		if ($event == $d['client'])
		{	
			if ($toUser != null)
				bufferevent_write($d['client'], $time . ' ' . $toUser . "\n");
		}
		else if ($toOthers != null)
			bufferevent_write($d['client'], $time . ' ' . $toOthers . "\n");
	}
}

function on_error($stream, $event, $code)
{
	global $CLIENTS;
	
	//echo "stream=$stream event=$event code=$code\n";
	if (($ip = findClientByResource($event)) == false)
	{
		echo "couldn't find client with stream '$event'\n";
		return;
	}
	
	if ($code | EVBUFFER_EOF)
	{
		echo "$ip ({$CLIENTS[$ip]['nick']}) disconnected (EOF).\n";
		unset($CLIENTS[$ip]);
	}
	else if ($code | EVBUFFER_ERROR)
	{
		echo "$ip ({$CLIENTS[$ip]['nick']}) client socket failed.\n";
		unset($CLIENTS[$ip]);
	}
	else
	{
		echo "$ip ({$CLIENTS[$ip]['nick']}) client socket died. no idea what happened.\n";
		unset($CLIENTS[$ip]);
	}
}

function on_write($event)
{
	//echo "on_write\n";
}

function on_sigint($signal, $flags, $event)
{
    global $CLIENTS;
    echo "on_sigint: You want me to stop, eh!?\n";
    echo "disconnecting clients...\n";
    foreach ($CLIENTS as $ip => $d)
    {
        echo "bye bye $ip" . (isset($d['nick']) ? (' (' . $d['nick'] . ')') : '') . "...";
        fclose($d['con']);
        echo "OK\n";
    }
    sleep(2);
    exit;
}

function on_accept($stream, $flags, $event)
{
	global $CLIENTS;
	global $count;
	$count++;
	$nick = "guest$count";
	$time = date('H:i:s');
	
	$mem = memory_get_usage();
    //echo "on_accept($stream, $flags): mem=$mem \n";
    if (false === ($con = stream_socket_accept($stream, 0, $ip)))
    	echo "Accept failed\n";
    else
    {
    	// notify server
    	echo "Connection from: $ip ($nick)\n";
    	
    	$client = bufferevent_new($con, 'on_read', 'on_write', 'on_error');
    	//debug_zval_dump($client);
        
    	// TODO: If bufferevent falls out of scope: disable and free buffer. also close stream.
        	
		// alert all other clients of new connection
    	foreach($CLIENTS as $c)
			bufferevent_write($c['client'], $time . " $nick has entered SamChat.\n");
        
    	$CLIENTS[$ip] = array(
    		'client'	=> &$client, 
    		'con'		=> &$con,
    		'nick'		=> $nick,
    		'admin'		=> false,
    	);
        bufferevent_enable($client, EV_READ);
    	
		bufferevent_write($client, "Welcome to SamChat $nick! Type '/HELP' for help.\n");
    }

}

if(!extension_loaded('event'))
{
	dl('event.' . PHP_SHLIB_SUFFIX);
}

event_init();

/* setup php stream */
$socket = stream_socket_server("tcp://0.0.0.0:8000", $errno, $errstr);
if (!$socket)
{
	echo "$errstr ($errno)\n";
	exit;
}

$event = event_new();
event_set($event, $socket, EV_READ | EV_PERSIST, 'on_accept' );
event_add($event);

$sigint = event_new();
event_set($sigint, SIGINT, EV_SIGNAL | EV_PERSIST, 'on_sigint');
event_add($sigint);

/* go! */
event_dispatch();


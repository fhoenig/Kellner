<?php
ini_set('include_path', ini_get('include_path').':../library');

require_once 'Zend/XmlRpc/Client.php';
require_once 'Zend/Http/Client.php';
require_once 'Zend/Json.php';

echo "Requesting through XML-RPC...\n";
$client = new Zend_XmlRpc_Client('http://127.0.0.1:8080/xmlrpc');

$result = $client->call('poll.getAllPolls');
print_r($result);

echo "Requesting though JSON-RPC...\n";
$json = '{"method":"poll.getAllPolls","params": []}';
$client = new Zend_Http_Client("http://127.0.0.1:8080/jsonrpc");
$client->setRawData($json);
$response = $client->request("POST");

print_r(Zend_Json::decode($response->getBody()));

<?php

if(!extension_loaded('event'))
{
    dl('event.' . PHP_SHLIB_SUFFIX);
}

class ApiServer
{
	private $addr = "127.0.0.1";
	private $port = 8080;
	private $httpd = null;
    private $controller = null;
    private $logger = null;
    private $base = null;
	private $config = null;
	private $xmlrpc_server = null;
	private $jsonrpc_server = null;
       
    public function __construct($addr="127.0.0.1", $port=8080)
    {
		$this->port = $port;
		$this->addr = $addr;
		
		// Setup basic stuff
        error_reporting(E_ALL & ~E_NOTICE);

		$base  = realpath(dirname(__FILE__) . '/../');
		$paths = array(
			'.', 
			realpath($base . '/library'),
			get_include_path(),
		);
		$this->base = $base;
		
		// Setup autoloading ...
		set_include_path(implode(PATH_SEPARATOR, $paths));
		require_once 'Zend/Loader/Autoloader.php';
		$autoloader = Zend_Loader_Autoloader::getInstance();
		$autoloader->registerNamespace('EvHttp_');
		
		$resourceLoader = new Zend_Loader_Autoloader_Resource(array(
		    'basePath'      => realpath($base . '/application'),
			'namespace'		=> '',
		    'resourceTypes' => array(
		        'model' => array(
					'namespace' => 'Model',
		            'path'      => 'models/',
		        ),
				'api' => array(
					'namespace' => 'Service',
		            'path'      => 'apis/',
		        ),
		    ),
		));
		
		// Load config
        $this->config = new Zend_Config_Ini($this->base . '/deploy/config.ini', 'default');
        Zend_Registry::set('config', $this->config);
		
		// Connect database
		$dbType = $this->config->database->type;
		$params = $this->config->database->toArray();
		$db = Zend_Db::factory($dbType, $params);
		Zend_Db_Table::setDefaultAdapter($db);
		Zend_Registry::set('db', $db);
		
		// Setup Xml-Rpc Server
		$this->xmlrpc_server = new Zend_XmlRpc_Server();
        $this->xmlrpc_server->setClass('Service_Poll', 'poll');
		Zend_XmlRpc_Server_Fault::attachFaultException('Exception');

		// Setup Json-Rpc Server
		$this->jsonrpc_server = new Zend_Json_Server();
        $this->jsonrpc_server->setClass('Service_Poll', 'poll')
        					 ->setAutoEmitResponse(false)
							 ->setTarget('/jsonrpc')
		           			 ->setEnvelope(Zend_Json_Server_Smd::ENV_JSONRPC_2);

    }
    
    public function processRequest($r)
    {
        try
        {
			printf("Used script-land memory %.2f MB\n", memory_get_usage()/(1024*1024));
			$uri = evhttp_request_get_uri($r);
			$parts = parse_url($uri);
			
			// let's process the request with the chosen webservice protocol
			switch (trim($parts['path'], "/"))
			{
				case "xmlrpc":
					// set custom request, since we have to get post data through another function
					$request = new EvHttp_XmlRpc_Request($r);
		            $this->xmlrpc_server->setRequest($request);
		            $response = $this->xmlrpc_server->handle();
		            unset($request);
					evhttp_response_add_header($r, "Content-Type", "text/xml");
					return evhttp_response_set($response->__toString(), 200, "OK");

				case "jsonrpc":
					if ("GET" == evhttp_request_method($r))
					{
						// Grab the SMD
					    $smd = $this->jsonrpc_server->getServiceMap();

					    // Return the SMD to the client
						evhttp_response_add_header($r, "Content-Type", "application/json");
						return evhttp_response_set($smd, 200, "OK");
					}
					else
					{
						// set custom request, since we have to get post data through another function
						$request = new EvHttp_Json_Request($r);
						$this->jsonrpc_server->setResponse(new Zend_Json_Server_Response());
		            	$response = $this->jsonrpc_server->handle($request);
			            unset($request);
						evhttp_response_add_header($r, "Content-Type", "application/json");
						return evhttp_response_set($response->toJson(), 200, "OK");
					}
				
				
				default:
					return evhttp_response_set("Invalid Protocol", 500, "Error");
			}
        }
        catch(Exception $e)
        {
        	return $e->__toString();			
        }
    }

	public function run()
	{
		event_init();
		$this->httpd = evhttp_start($this->addr, $this->port);
		evhttp_set_gencb($this->httpd, array($this, 'processRequest'));
		echo "XML-RPC server started at http://{$this->addr}:{$this->port}/xmlrpc...\n";
		event_dispatch();
	}
}


$app = new ApiServer("0.0.0.0", 8080);
$app->run();


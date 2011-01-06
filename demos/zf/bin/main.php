<?php

if(!extension_loaded('event'))
{
    dl('event.' . PHP_SHLIB_SUFFIX);
}

class AppServer
{
	private $addr = "127.0.0.1";
	private $port = 8080;
	private $httpd = null;
    private $controller = null;
    private $logger = null;
    private $base = null;
	private $config = null;


	private function _startupSuperGlobal()
	{
		// manually set server vars
		$_SERVER[ "DOCUMENT_ROOT" ] = realpath(dirname(__FILE__) . '/../public');
        $_SERVER[ "SCRIPT_FILENAME" ] = "main.php";
        $_SERVER[ "PHP_SELF" ] = "main.php";
        $_SERVER[ "SCRIPT_NAME" ] = 'main.php';
        $_SERVER[ "SERVER_ADDR" ] = "{$this->addr}:{$this->port}";
        $_SERVER[ "SERVER_NAME" ] = "{$this->addr}:{$this->port}";
        $_SERVER[ "SERVER_SOFTWARE" ] = "EvHttp ZF Application Server 0.1";
		$_SERVER[ "SERVER_PROTOCOL" ] = 'HTTP 1.1';
	}

    public function __construct($addr="127.0.0.1", $port=8080)
    {
		$this->port = $port;
		$this->addr = $addr;
		$this->_startupSuperGlobal();

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
				'form' => array(
					'namespace' => 'Form',
		            'path'      => 'forms/',
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

		// Setup extended front controller.
		$this->controller = EvHttp_Controller_Front::getInstance();
		$this->controller->throwExceptions(true);
		$this->controller->returnResponse(true);
		$this->controller->setDefaultAction('index');
		$this->controller->setControllerDirectory(array(
			'default' 		=> $base . '/application/controllers'
		));
    }

	private function _initRequest($r)
	{
		$_SERVER[ "REQUEST_METHOD" ] = evhttp_request_method($r);
        $_SERVER[ "REQUEST_TIME" ] = time();
        $_SERVER[ "argv" ] = $_SERVER[ "REQUEST_URI" ] = evhttp_request_get_uri($r);

		$parts = parse_url($_SERVER["REQUEST_URI"]);
		$_SERVER['QUERY_STRING'] = $parts['query'];
		parse_str($_SERVER['QUERY_STRING'], $_GET);

		$headers = evhttp_request_headers($r);
		// normalize to php way
		foreach ($headers as $name => $value)
			$_SERVER["HTTP_" . str_replace("-", "_", strtoupper($name))] = $value;

		echo "script-land memory: ".memory_get_usage()."\n";
	}

    public function processRequest($r)
    {
        try
        {
			$this->_initRequest($r);
			$response = $this->controller->dispatch();
        }
        catch(Exception $e)
        {
        	return $e->__toString();
        }
		return evhttp_response_set($response->getBody(), $response->getHttpResponseCode(), "OK");
    }

	public function run()
	{
		event_init();
		$this->httpd = evhttp_start($this->addr, $this->port);
		evhttp_set_gencb($this->httpd, array($this, 'processRequest'));
		echo "Application listening at {$this->addr}:{$this->port}...\n";
		event_dispatch();
	}
}


$app = new AppServer("0.0.0.0", 8080);
$app->run();


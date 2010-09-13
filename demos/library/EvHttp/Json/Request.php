<?php

require_once 'Zend/Json/Server/Request.php';

class EvHttp_Json_Request extends Zend_Json_Server_Request
{
    /**
     * Raw JSON pulled from POST body
     * @var string
     */
    protected $_rawJson;

    /**
     * Constructor
     *
     * Pull JSON request from raw POST body and use to populate request.
     * 
     * @return void
     */
    public function __construct($evhttp_request)
    {
        $json = evhttp_request_body($evhttp_request);
        $this->_rawJson = $json;

        if (!empty($json)) {
            $this->loadJson($json);
        }
    }

    /**
     * Get JSON from raw POST body
     * 
     * @return string
     */
    public function getRawJson()
    {
        return $this->_rawJson;
    }
}

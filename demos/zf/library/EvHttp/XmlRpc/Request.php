<?php

/**
 * Zend_XmlRpc_Request
 */
require_once 'Zend/XmlRpc/Request.php';


class EvHttp_XmlRpc_Request extends Zend_XmlRpc_Request
{
    /**
     * Raw XML as received via request
     * @var string
     */
    protected $_xml;

    /**
     * Constructor
     *
     * Attempts to read from evhttp_request resource the raw POST request; if an error
     * occurs in doing so, or if the XML is invalid, the request is declared a
     * fault.
     *
     * @return void
     */
    public function __construct($evhttp_request)
    {
        if (($this->_xml = evhttp_request_body($evhttp_request)) == FALSE)
        {
            $this->_fault = new Zend_XmlRpc_Server_Exception(630);
            return;
        }
        $this->loadXml($this->_xml);
    }

    /**
     * Retrieve the raw XML request
     *
     * @return string
     */
    public function getRawRequest()
    {
        return $this->_xml;
    }
}

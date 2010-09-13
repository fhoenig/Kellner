<?php
/**
 * Zend Framework
 *
 * LICENSE
 *
 * This source file is subject to the new BSD license that is bundled
 * with this package in the file LICENSE.txt.
 * It is also available through the world-wide-web at this URL:
 * http://framework.zend.com/license/new-bsd
 * If you did not receive a copy of the license and are unable to
 * obtain it through the world-wide-web, please send an email
 * to license@zend.com so we can send you a copy immediately.
 *
 * @category   Zend
 * @package    EvHttp_Controller
 * @subpackage Plugins
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 */

/** EvHttp_Controller_Plugin_Abstract */
require_once 'EvHttp/Controller/Plugin/Abstract.php';

/** Zend_Registry */
require_once 'Zend/Registry.php';

/**
 * Manage a stack of actions
 *
 * @uses       EvHttp_Controller_Plugin_Abstract
 * @category   Zend
 * @package    EvHttp_Controller
 * @subpackage Plugins
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 * @version    $Id: ActionStack.php 8064 2008-02-16 10:58:39Z thomas $
 */
class EvHttp_Controller_Plugin_ActionStack extends EvHttp_Controller_Plugin_Abstract
{
    /** @var Zend_Registry */
    protected $_registry;

    /**
     * Registry key under which actions are stored
     * @var string
     */
    protected $_registryKey = 'EvHttp_Controller_Plugin_ActionStack';

    /**
     * Valid keys for stack items
     * @var array
     */
    protected $_validKeys = array(
        'module', 
        'controller',
        'action',
        'params'
    );

    /**
     * Constructor
     *
     * @param  Zend_Registry $registry
     * @param  string $key
     * @return void
     */
    public function __construct(Zend_Registry $registry = null, $key = null)
    {
        if (null === $registry) {
            $registry = Zend_Registry::getInstance();
        }
        $this->setRegistry($registry);

        if (null !== $key) {
            $this->setRegistryKey($key);
        } else {
            $key = $this->getRegistryKey();
        }

        $registry[$key] = array();
    }

    /**
     * Set registry object
     * 
     * @param  Zend_Registry $registry 
     * @return EvHttp_Controller_Plugin_ActionStack
     */
    public function setRegistry(Zend_Registry $registry)
    {
        $this->_registry = $registry;
        return $this;
    }

    /**
     * Retrieve registry object
     * 
     * @return Zend_Registry
     */
    public function getRegistry()
    {
        return $this->_registry;
    }

    /**
     * Retrieve registry key
     *
     * @return string
     */
    public function getRegistryKey()
    {
        return $this->_registryKey;
    }

    /**
     * Set registry key
     *
     * @param  string $key
     * @return EvHttp_Controller_Plugin_ActionStack
     */
    public function setRegistryKey($key)
    {
        $this->_registryKey = (string) $key;
        return $this;
    }

    /**
     * Retrieve action stack
     * 
     * @return array
     */
    public function getStack()
    {
        $registry = $this->getRegistry();
        $stack    = $registry[$this->getRegistryKey()];
        return $stack;
    }

    /**
     * Save stack to registry
     * 
     * @param  array $stack 
     * @return EvHttp_Controller_Plugin_ActionStack
     */
    protected function _saveStack(array $stack)
    {
        $registry = $this->getRegistry();
        $registry[$this->getRegistryKey()] = $stack;
        return $this;
    }

    /**
     * Push an item onto the stack
     * 
     * @param  EvHttp_Controller_Request_Abstract $next 
     * @return EvHttp_Controller_Plugin_ActionStack
     */
    public function pushStack(EvHttp_Controller_Request_Abstract $next)
    {
        $stack = $this->getStack();
        array_push($stack, $next);
        return $this->_saveStack($stack);
    }

    /**
     * Pop an item off the action stack
     * 
     * @return false|EvHttp_Controller_Request_Abstract
     */
    public function popStack()
    {
        $stack = $this->getStack();
        if (0 == count($stack)) {
            return false;
        }

        $next = array_pop($stack);
        $this->_saveStack($stack);

        if (!$next instanceof EvHttp_Controller_Request_Abstract) {
            require_once 'EvHttp/Controller/Exception.php';
            throw new EvHttp_Controller_Exception('ArrayStack should only contain request objects');
        }
        $action = $next->getActionName();
        if (empty($action)) {
            return $this->popStack($stack);
        }

        $request    = $this->getRequest();
        $controller = $next->getControllerName();
        if (empty($controller)) {
            $next->setControllerName($request->getControllerName());
        }

        $module = $next->getModuleName();
        if (empty($module)) {
            $next->setModuleName($request->getModuleName());
        }

        return $next;
    }

    /**
     * postDispatch() plugin hook -- check for actions in stack, and dispatch if any found
     *
     * @param  EvHttp_Controller_Request_Abstract $request
     * @return void
     */
    public function postDispatch(EvHttp_Controller_Request_Abstract $request)
    {
        // Don't move on to next request if this is already an attempt to 
        // forward
        if (!$request->isDispatched()) {
            return;
        }

        $this->setRequest($request);
        $stack = $this->getStack();
        if (empty($stack)) {
            return;
        }
        $next = $this->popStack();
        if (!$next) {
            return;
        }

        $this->forward($next);
    }

    /**
     * Forward request with next action
     * 
     * @param  array $next 
     * @return void
     */
    public function forward(EvHttp_Controller_Request_Abstract $next)
    {
        $this->getRequest()->setModuleName($next->getModuleName())
                           ->setControllerName($next->getControllerName())
                           ->setActionName($next->getActionName())
                           ->setParams($next->getParams())
                           ->setDispatched(false);
    }
}

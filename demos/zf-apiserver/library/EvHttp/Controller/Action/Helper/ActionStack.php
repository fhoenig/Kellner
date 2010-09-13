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
 * @subpackage EvHttp_Controller_Action_Helper
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 * @version    $Id: ActionStack.php 11493 2008-09-23 14:25:11Z doctorrock83 $
 */

/**
 * @see EvHttp_Controller_Action_Helper_Abstract
 */
require_once 'EvHttp/Controller/Action/Helper/Abstract.php';

/**
 * Add to action stack
 *
 * @uses       EvHttp_Controller_Action_Helper_Abstract
 * @category   Zend
 * @package    EvHttp_Controller
 * @subpackage EvHttp_Controller_Action_Helper
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 */
class EvHttp_Controller_Action_Helper_ActionStack extends EvHttp_Controller_Action_Helper_Abstract
{
    /**
     * @var EvHttp_Controller_Plugin_ActionStack
     */
    protected $_actionStack;

    /**
     * Constructor
     *
     * Register action stack plugin
     * 
     * @return void
     */
    public function __construct()
    {
        $front = EvHttp_Controller_Front::getInstance();
        if (!$front->hasPlugin('EvHttp_Controller_Plugin_ActionStack')) {
            /**
             * @see EvHttp_Controller_Plugin_ActionStack
             */
            require_once 'EvHttp/Controller/Plugin/ActionStack.php';
            $this->_actionStack = new EvHttp_Controller_Plugin_ActionStack();
            $front->registerPlugin($this->_actionStack, 97);
        } else {
            $this->_actionStack = $front->getPlugin('EvHttp_Controller_Plugin_ActionStack');
        }
    }

    /**
     * Push onto the stack 
     * 
     * @param  EvHttp_Controller_Request_Abstract $next 
     * @return EvHttp_Controller_Action_Helper_ActionStack Provides a fluent interface
     */
    public function pushStack(EvHttp_Controller_Request_Abstract $next)
    {
        $this->_actionStack->pushStack($next);
        return $this;
    }

    /**
     * Push a new action onto the stack
     * 
     * @param  string $action 
     * @param  string $controller 
     * @param  string $module 
     * @param  array  $params
     * @throws EvHttp_Controller_Action_Exception 
     * @return EvHttp_Controller_Action_Helper_ActionStack
     */
    public function actionToStack($action, $controller = null, $module = null, array $params = array())
    {
        if ($action instanceof EvHttp_Controller_Request_Abstract) {
            return $this->pushStack($action);
        } elseif (!is_string($action)) {
            /**
             * @see EvHttp_Controller_Action_Exception
             */
            require_once 'EvHttp/Controller/Action/Exception.php';
            throw new EvHttp_Controller_Action_Exception('ActionStack requires either a request object or minimally a string action');
        }

        $request = $this->getRequest();

        if ($request instanceof EvHttp_Controller_Request_Abstract === false){
            /**
             * @see EvHttp_Controller_Action_Exception
             */
            require_once 'EvHttp/Controller/Action/Exception.php';
            throw new EvHttp_Controller_Action_Exception('Request object not set yet');
        }
        
        $controller = (null === $controller) ? $request->getControllerName() : $controller;
        $module = (null === $module) ? $request->getModuleName() : $module;

        /**
         * @see EvHttp_Controller_Request_Simple
         */
        require_once 'EvHttp/Controller/Request/Simple.php';
        $newRequest = new EvHttp_Controller_Request_Simple($action, $controller, $module, $params);

        return $this->pushStack($newRequest);
    }

    /**
     * Perform helper when called as $this->_helper->actionStack() from an action controller
     *
     * Proxies to {@link simple()}
     *
     * @param  string $action
     * @param  string $controller
     * @param  string $module
     * @param  array $params
     * @return boolean
     */
    public function direct($action, $controller = null, $module = null, array $params = array())
    {
        return $this->actionToStack($action, $controller, $module, $params);
    }
}

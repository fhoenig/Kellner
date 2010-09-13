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
 * @package    App_Controller
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 */


/** Ev Http Controller Action */
require_once 'EvHttp/Controller/Action.php';


/**
 * @category   Zend
 * @package    App_Controller
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 */
class App_Controller_Default extends EvHttp_Controller_Action
{
    public function init()
    {
        parent::init();
         
        if (!Zend_Session::isStarted())
        {
            // start session
            $config = Zend_Registry::get('config');
            $session = $config->session->toArray();
            $session['save_path'] = realpath($session['save_path']);
            Zend_Session::setOptions($session);
            Zend_Session::start();
        }
    }
	
	public function preDispatch()
	{
        if (Zend_Auth::getInstance()->hasIdentity()) {
            // If the user is logged in, we don't want to show the login form;
            // however, the logout action should still be available
            if ('logout' != $this->getRequest()->getActionName()) {
                die('should be on poll page');
                //$this->_helper->redirector('poll', 'index');
            }
        } else {
            // If they aren't, they can't logout, so that action should
            // redirect to the login form
            if ('logout' == $this->getRequest()->getActionName()) {
                die('should be on login page');
                //$this->_helper->redirector('index');
            }
        }
	}
}
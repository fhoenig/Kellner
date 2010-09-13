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
 * @package    App_Auth
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 */


/** Zend Auth Adapter Interface */
require_once 'Zend/Auth/Adapter/Interface.php';


/**
 * @category   Zend
 * @package    App_Auth
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 */
class App_Auth_Adapter extends Zend_Auth_Adapter_Interface
{
    private $_name = null;
    private $_password = null;
    
    /**
     * create new auth adapter
     *
     * @param string $name
     * @param string $password
     */
    public function __construct($name, $password)
    {
        $this->_name = $name;
        $this->_password = $password;
    }
    
    
    public function authenticate()
    {
        if ($user = Model_Table_User::instance()->authenticate($this->_name, $this->_password))
            return new Zend_Auth_Result(1, $this->_name);
        else
            return new Zend_Auth_Result(-3, null);
    }
}
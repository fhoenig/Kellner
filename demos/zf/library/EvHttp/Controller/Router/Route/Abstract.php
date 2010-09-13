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
 * @package    EvHttp_Controller
 * @subpackage Router
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @version    $Id: Route.php 1847 2006-11-23 11:36:41Z martel $
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 */

/**
 * @see EvHttp_Controller_Router_Route_Interface
 */
require_once 'EvHttp/Controller/Router/Route/Interface.php';

/**
 * Abstract Route
 *
 * Implements interface and provides convenience methods
 *
 * @package    EvHttp_Controller
 * @subpackage Router
 * @copyright  Copyright (c) 2005-2008 Zend Technologies USA Inc. (http://www.zend.com)
 * @license    http://framework.zend.com/license/new-bsd     New BSD License
 */
abstract class EvHttp_Controller_Router_Route_Abstract implements EvHttp_Controller_Router_Route_Interface
{
    /**
     * Wether this route is abstract or not
     *
     * @var boolean
     */
    protected $_isAbstract = false;

    /**
     * Path matched by this route
     *
     * @var string
     */
    protected $_matchedPath = null;
    
    /**
     * Get the version of the route
     *
     * @return integer
     */
    public function getVersion()
    {
        return 2;
    }
    
    /**
     * Set partially matched path
     *
     * @param  string $path
     * @return void
     */
    public function setMatchedPath($path)
    {
        $this->_matchedPath = $path;
    }
    
    /**
     * Get partially matched path
     *
     * @return string
     */
    public function getMatchedPath()
    {
        return $this->_matchedPath;
    }
    
    /**
     * Check or set wether this is an abstract route or not
     * 
     * @param  boolean $flag
     * @return boolean
     */
    public function isAbstract($flag = null)
    {
        if ($flag !== null) {
            $this->_isAbstract = $flag;
        }
    
        return $this->_isAbstract;
    }
    
    /**
     * Create a new chain
     * 
     * @param  EvHttp_Controller_Router_Route_Abstract $route
     * @param  string                                $separator
     * @return EvHttp_Controller_Router_Route_Chain
     */    
    public function chain(EvHttp_Controller_Router_Route_Abstract $route, $separator = '/')
    {
        require_once 'EvHttp/Controller/Router/Route/Chain.php';

        $chain = new EvHttp_Controller_Router_Route_Chain();
        $chain->chain($this)->chain($route, $separator);

        return $chain;
    }

}

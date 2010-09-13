<?php

/**
 * User
 *
 * @package Model
 * @subpackage Table
 * @author Florian Hoenig
 **/
class Model_Table_User extends Zend_Db_Table_Abstract
{
	protected $_name = 'user';
	protected $_primary = 'id';
	protected $_dependentTables = array('Model_Table_Poll_Choice');
		
	static protected $_instance = null;
	protected function __clone(){}
	
	/**
	 * Return one and only one instance of the object
	 */
	static public function instance()
	{
		if (!self::$_instance instanceof self)
			self::$_instance = new self();
		return self::$_instance;
	}
	
}
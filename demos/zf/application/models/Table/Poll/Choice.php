<?php

/**
 * Poll
 *
 * @package Model
 * @subpackage Table
 * @author Florian Hoenig
 **/
class Model_Table_Poll_Choice extends Zend_Db_Table_Abstract
{
	protected $_name = 'poll_choice';
	protected $_primary = 'id';
	
	protected $_referenceMap    = array(
		'Voter' => array(
			'columns'           => 'user_id',
			'refTableClass'     => 'Model_Table_User',
			'refColumns'        => 'id'
		),
		'Choice' => array(
			'columns'           => 'poll_option_id',
			'refTableClass'     => 'Model_Table_Poll_Option',
			'refColumns'        => 'id'
		)
	);
	
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



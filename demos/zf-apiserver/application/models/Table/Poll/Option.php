<?php

/**
 * Poll
 *
 * @package Model
 * @subpackage Table
 * @author Florian Hoenig
 **/
class Model_Table_Poll_Option extends Zend_Db_Table_Abstract
{
	protected $_name = 'poll_option';
	protected $_primary = 'id';
	protected $_dependentTables = array('Model_Table_Poll_Choice');
	
	protected $_referenceMap    = array(
		'Poll' => array(
			'columns'           => 'poll_id',
			'refTableClass'     => 'Model_Table_Poll',
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
		
	public function findOption($poll, $key)
	{
	    $now = date('Y-m-d H:i:s');
		$db = $this->getAdapter();
		$where = $db->quoteInto('poll_id = ?', $poll->id)
			   . $db->quoteInto(" AND UPPER(`key`) LIKE UPPER(?)", $key);
			   
		return $this->fetchRow($where, null);
	}
	
	public function findForPoll($poll)
	{
		$db = $this->getAdapter();
		$where = $db->quoteInto('poll_id = ?', $poll->id);
		return $this->fetchAll($where, "key");
	}
}



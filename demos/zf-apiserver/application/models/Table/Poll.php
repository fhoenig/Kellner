<?php

/**
 * Poll
 *
 * @package Model
 * @subpackage Table
 * @author Florian Hoenig
 **/
class Model_Table_Poll extends Zend_Db_Table_Abstract
{
	protected $_name = 'poll';
	protected $_primary = 'id';
	protected $_dependentTables = array('Model_Table_Poll_Option');
	
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

	public function vote($mobileuser, $poll, $arg)
	{
	    if (!($poll_option = Model_Poll_Option::instance()->findOption($poll, $arg)))
	        return false;

	    try
	    {
    	    $choice = Model_Poll_Choice::instance()->fetchNew();
    	    $choice->mobileuser_id = $mobileuser->id;
    	    $choice->poll_option_id = $poll_option->id;
    	    $choice->save();
    	    return true;
	    }
	    catch (Zend_Db_Exeption $e)
	    {
	        return false;
	    }
	}
	
	public function getResults($poll)
	{
	    $db = $this->getAdapter();
		$select = $db->select();
		
		$select->from('poll_option', array('text', 'key'));
		$select->joinLeft('poll_choice', 'poll_option.id = poll_choice.poll_option_id', array("count(mobileuser_id) as votes"));
		$select->where("poll_id = ?", $poll->id);
		$select->group("key");

		$stmt = $select->query();
		return $stmt->fetchAll();
	}
}



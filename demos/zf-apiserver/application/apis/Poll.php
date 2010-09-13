<?php

class Service_Poll
{
	/**
    * Retrieve all poll questions
    *
    * @return array Questions
    */
	public static function getAllQuestions()
	{
		$polls = Model_Table_Poll::instance()->fetchAll();

		$questions = array();
		foreach ($polls as $poll)
			$questions[] = $poll->question;
			
		return $questions;
	}
	
	/**
    * Retrieve all polls
    *
    * @return struct polls
    */
	public static function getAllPolls()
	{
		$polls = Model_Table_Poll::instance()->fetchAll();

		$ret = array();
		foreach ($polls as $poll)
			foreach($poll->findDependentRowset('Model_Table_Poll_Option') as $option)
				$ret[$poll->question][$option->key] = $option->text;
							
		return $ret;
	}
	
}
<?php

class PollController extends EvHttp_Controller_Action
{
	
    public function init()
    {
        /* Initialize action controller here */
    }

    public function indexAction()
    {
        $polls = Model_Table_Poll::instance()->fetchAll();

		$this->view->polls = array();
		
		foreach ($polls as $poll)
		{
			$options = $poll->findDependentRowset('Model_Table_Poll_Option');
			$this->view->polls[$poll->id]['question'] = $poll->question;
			$this->view->polls[$poll->id]['options'] = $options->toArray();
		}
    }


}


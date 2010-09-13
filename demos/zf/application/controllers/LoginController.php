<?php

class LoginController extends App_Controller_Default
{
	protected $_loginForm = null;

    public function init()
    {
        $this->_loginForm = new Form_Login();
		$this->_loginForm->setAction("/login/auth");
    }

    public function indexAction()
    {
		$this->view->loginForm = $this->_loginForm;
    }
    
    public function getAuthAdapter(array $params)
    {
        // Leaving this to the developer...
        // Makes the assumption that the constructor takes an array of
        // parameters which it then uses as credentials to verify identity.
        // Our form, of course, will just pass the parameters 'username'
        // and 'password'.
        return new App_Auth_Adapter($params['username'], $params['password']);
    }
    
    public function authAction()
    {
        $request = $this->getRequest();

        // Check if we have a POST request
        if (!$request->isPost()) {
            die('not posted');
            //return $this->_helper->redirector('index');
        }

        // Get our form and validate it
        $form = $this->_loginForm;
        if (!$form->isValid($request->getPost())) {
            // Invalid entries
            $this->view->loginForm = $form;
            return $this->render('index'); // re-render the login form
        }

        // Get our authentication adapter and check credentials
        $adapter = $this->getAuthAdapter($form->getValues());
        $auth    = Zend_Auth::getInstance();
        $result  = $auth->authenticate($adapter);
        if (!$result->isValid()) {
            // Invalid credentials
            $form->setDescription('Invalid credentials provided');
            $this->view->loginForm = $form;
            return $this->render('index'); // re-render the login form
        }

        die("We're authenticated! Redirect to the home page");
        //$this->_helper->redirector('poll', 'index');
    }
}

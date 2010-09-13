<?php

class Form_Login extends Zend_Form
{
    public function init()
    {
        $username = new Zend_Form_Element_Text('username');
        $username->class = 'formtext';
        $username->setLabel('Username:')
                 ->setDecorators(array(
                     array('ViewHelper',
                           array('helper' => 'formText')),
                     array('Label',
                           array('class' => 'label'))
                 ));

        $password = new Zend_Form_Element_Password('password');
        $password->class = 'formtext';
        $password->setLabel('Password:')
                 ->setDecorators(array(
                     array('ViewHelper',
                           array('helper' => 'formPassword')),
                     array('Label',
                           array('class' => 'label'))
                 ));

        $submit = new Zend_Form_Element_Submit('login');
        $submit->class = 'formsubmit';
        $submit->setValue('Login')
               ->setDecorators(array(
                   array('ViewHelper',
                   array('helper' => 'formSubmit'))
               ));

        $this->addElements(array(
            $username,
            $password,
            $submit
        ));

        $this->setDecorators(array(
            'FormElements',
            'Fieldset',
            'Form'
        ));
    }
}
<?php

class Form_Poll extends Zend_Form
{
    public function init()
    {
        $submit = new Zend_Form_Element_Submit('submit');
        $submit->class = 'formsubmit';
        $submit->setValue('Submit')
               ->setDecorators(array(
                   array('ViewHelper',
                   array('helper' => 'formSubmit'))
               ));

        $this->addElements(array(
            $submit
        ));

        $this->setDecorators(array(
            'FormElements',
            'Fieldset',
            'Form'
        ));
    }

	public function addOption($option)
	{
		$o = new Zend_Form_Element_Radio('submit');
        $o->class = 'formsubmit';
        $o->setValue('Submit')
          ->setDecorators(array(
               array('ViewHelper',
               array('helper' => 'formSubmit'))
          ));
	}
}
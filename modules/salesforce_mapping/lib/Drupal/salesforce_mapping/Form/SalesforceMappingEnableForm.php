<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\SalesforceMappingEnableForm.
 */

namespace Drupal\salesforce_mapping\Form;

use Drupal\Core\Entity\EntityConfirmFormBase;

/**
 * Salesforce Mapping Disable Form .
 */
class SalesforceMappingEnableForm extends EntityConfirmFormBase {

  /**
   * {@inheritdoc}
   */
  public function getQuestion() {
    return $this->t('Are you sure you want to enable the mapping %name?', array('%name' => $this->entity->label()));
  }

  /**
   * {@inheritdoc}
   */
  public function getDescription() {
    return $this->t('Enabling a mapping will restart any automatic synchronization.');
  }

  /**
   * {@inheritdoc}
   */
  public function getCancelRoute() {
    return array(
      'route_name' => 'salesforce_mapping.list',
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getConfirmText() {
    return $this->t('Enable');
  }

  /**
   * {@inheritdoc}
   */
  public function submit(array $form, array &$form_state) {
    parent::submit($form, $form_state);

    $this->entity->enable()->save();
    $form_state['redirect_route'] = array(
      'route_name' => 'salesforce_mapping.edit',
      'route_parameters' => array('salesforce_mapping' => $this->entity->id()),
    );
  }

}

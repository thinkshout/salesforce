<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\SalesforceMappingDisableForm.
 */

namespace Drupal\salesforce_mapping\Form;

use Drupal\Core\Entity\EntityConfirmFormBase;

/**
 * Salesforce Mapping Disable Form .
 */
class SalesforceMappingDisableForm extends EntityConfirmFormBase {

  /**
   * {@inheritdoc}
   */
  public function getQuestion() {
    return $this->t('Are you sure you want to disable the mapping %name?', array('%name' => $this->entity->label()));
  }

  /**
   * {@inheritdoc}
   */
  public function getDescription() {
    return $this->t('Disabling a mapping will stop any automatic synchronization and hide the mapping.');
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
    return $this->t('Disable');
  }

  /**
   * {@inheritdoc}
   */
  public function submit(array $form, array &$form_state) {
    parent::submit($form, $form_state);

    $this->entity->disable()->save();
     $form_state['redirect_route'] = array(
      'route_name' => 'salesforce_mapping.list',
    );
  }

}

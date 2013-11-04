<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\SalesforceMappingDeleteForm.
 */

namespace Drupal\salesforce_mapping\Form;

use Drupal\Core\Entity\EntityConfirmFormBase;

/**
 * Salesforce Mapping Delete Form .
 */
class SalesforceMappingDeleteForm extends EntityConfirmFormBase {

  /**
   * {@inheritdoc}
   */
  public function getQuestion() {
    return $this->t('Are you sure you want to delete the mapping %name?', array('%name' => $this->entity->label()));
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
    return $this->t('Delete');
  }

  /**
   * {@inheritdoc}
   */
  public function submit(array $form, array &$form_state) {
    parent::submit($form, $form_state);

    $this->entity->delete();
     $form_state['redirect_route'] = array(
      'route_name' => 'salesforce_mapping.list',
    );
  }

}

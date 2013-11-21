<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field\Properties.
 */

namespace Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\Entity\EntityInterface;
use Drupal\salesforce_mapping\Plugin\FieldPluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
// use Drupal\Core\Utility\Token;
use Symfony\Component\DependencyInjection\ContainerInterface;


/**
 * Adapter for entity properties and fields.
 *
 * @Plugin(
 *   id = "properties",
 *   label = @Translation("Properties")
 * )
 */
class Properties extends FieldPluginBase {

  public function buildConfigurationForm(array $form, array &$form_state) {
    // @todo inspecting the form and form_state feels wrong, but haven't found a good way to get the entity from config before the config is saved.
    $entity = $form['#entity'];
    $entity_info = $this->entityManager->getFieldDefinitions($entity->get('drupal_entity_type'), $entity->get('drupal_bundle'));
    // Discard all but keys and labels:
    array_walk($entity_info, function(&$value, $key) {
      $value = $value['label'];
    });
    return array(
      '#type' => 'select',
      '#options' => $entity_info,
      '#empty_option' => $this->t('- Select -'),
      '#default_value' => $this->config('drupal_field_value'),
      '#description' => $this->t('Select a Drupal field or property to map to a Salesforce field. Related are left out and should be handled using another fieldmap type like tokens.'),
    );
  }

  public function value(EntityInterface $entity) {
    // No error checking here. If a property is not defined, it's a
    // configuration bug that needs to be solved elsewhere.
    return $entity->get($this->config('drupal_field_value'))->value;
  }

}
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

/**
 * Adapter for entity properties and fields.
 *
 * @Plugin(
 *   id = "properties",
 *   label = @Translation("Properties")
 * )
 */
class Properties extends FieldPluginBase {

  /**
   * Implementation of PluginFormInterface::buildConfigurationForm
   */
  public function buildConfigurationForm(array $form, array &$form_state) {
    // @todo inspecting the form and form_state feels wrong, but haven't found a good way to get the entity from config before the config is saved.
    $entity = $form['#entity'];
    $entity_info = $this->entityManager->getFieldDefinitions($entity->get('drupal_entity_type'), $entity->get('drupal_bundle'));

    // Discard all but keys and labels:
    array_walk($entity_info, function(&$value, $key) {
      // Entity References are handled separately.
      // @todo is there a better way to identify entity reference fields?
      if ($value['type'] == 'field_item:entity_reference') {
        $value = '';
      }
      else {
        $value = $value['label'];
      }
    });
    $entity_info = array_filter($entity_info);
    return array(
      '#type' => 'select',
      '#options' => $entity_info,
      '#empty_option' => $this->t('- Select -'),
      '#default_value' => $this->config('drupal_field_value'),
      '#description' => $this->t('Select a Drupal field or property to map to a Salesforce field.<br />Entity Reference fields should be handled using Related Entity Ids or Token field types.'),
    );
  }

  public function value(EntityInterface $entity) {
    // No error checking here. If a property is not defined, it's a
    // configuration bug that needs to be solved elsewhere.
    return $entity->get($this->config('drupal_field_value'))->value;
  }

}
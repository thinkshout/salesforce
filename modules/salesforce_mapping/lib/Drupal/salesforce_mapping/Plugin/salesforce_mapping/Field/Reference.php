<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field\Reference.
 */

namespace Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\Entity\EntityInterface;
use Drupal\salesforce_mapping\Plugin\FieldPluginBase;

/**
 * Adapter for entity Reference and fields.
 *
 * @Plugin(
 *   id = "Reference",
 *   label = @Translation("Related Entity Ids")
 * )
 */
class Reference extends FieldPluginBase {

  /**
   * Implementation of PluginFormInterface::buildConfigurationForm
   * This is basically the inverse of Properties::buildConfigurationForm()
   */
  public function buildConfigurationForm(array $form, array &$form_state) {
    // @todo inspecting the form and form_state feels wrong, but haven't found a good way to get the entity from config before the config is saved.
    $mapping = $form['#entity'];
    $entity_info = $this->entityManager->getFieldDefinitions($mapping->get('drupal_entity_type'), $mapping->get('drupal_bundle'));

    // Discard all but keys and labels:
    array_walk($entity_info, function(&$value, $key) {
      // @todo is there a better way to identify entity reference fields?
      if ($value['type'] != 'field_item:entity_reference') {
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
      '#description' => $this->t('If an existing connection is found with the selected entity reference, the linked identifier will be used.<br />For example, Salesforce ID for Drupal to SF, or Node ID for SF to Drupal.'),
    );
  }

  public function value(EntityInterface $entity) {
    // No error checking here. If a property is not defined, it's a
    // configuration bug that needs to be solved elsewhere.
    $field = $entity->get($this->config('drupal_field_value'));
    $field_settings =  $field->getFieldDefinition()->getFieldSettings();
    $referenced_entity = $this->entityManager
      ->getStorageController($field_settings['target_type'])
      ->load($field->value);
    // @todo this procedural call will go away when sf mapping object becomes a service or field
    if ($referenced_mapping =
      salesforce_mapping_object_load_by_entity($referenced_entity)) {
      return $referenced_mapping->sfid();
    }
  }

}
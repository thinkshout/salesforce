<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce\MappingField\RelatedIDs.
 */

namespace Drupal\salesforce_mapping\Plugin\salesforce\MappingField;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\Entity\EntityInterface;
use Drupal\field\Field;
use Drupal\salesforce_mapping\Plugin\MappingFieldPluginBase;

/**
 * Adapter for entity Reference and fields.
 *
 * @Plugin(
 *   id = "RelatedIDs",
 *   label = @Translation("Related Entity Ids")
 * )
 */
class RelatedIDs extends MappingFieldPluginBase {

  /**
   * Implementation of PluginFormInterface::buildConfigurationForm
   * This is basically the inverse of Properties::buildConfigurationForm()
   */
  public function buildConfigurationForm(array $form, array &$form_state) {
    // @todo inspecting the form and form_state feels wrong, but haven't found a good way to get the entity from config before the config is saved.
    $options = $this->getConfigurationOptions($form['#entity']);

    if (empty($options)) {
      return array(
        '#markup' => t('No available entity reference fields.')
      );
    }
    return array(
      '#type' => 'select',
      '#options' => $options,
      '#empty_option' => $this->t('- Select -'),
      '#default_value' => $this->config('drupal_field_value'),
      '#description' => $this->t('If an existing connection is found with the selected entity reference, the linked identifier will be used.<br />For example, Salesforce ID for Drupal to SF, or Node ID for SF to Drupal.<br />If more than one entity is referenced, the entity at delta zero will be used.'),
    );
  }

  /**
   * @see RelatedProperties::value
   */
  public function value(EntityInterface $entity) {
    $field_name = $this->config('drupal_field_value');
    $instances = Field::fieldInfo()->getBundleInstances(
      $entity->entityType(),
      $entity->bundle()
    );
    if (empty($instances[$field_name])) {
      return;
    }

    $field = $entity->get($field_name);
    if (empty($field->value)) {
      // This reference field is blank
      return;
    }

    // Now we can actually fetch the referenced entity.
    $field_settings = $field->getFieldDefinition()->getFieldSettings();
    // @todo this procedural call will go away when sf mapping object becomes a service or field
    if ($referenced_mapping =
      salesforce_mapping_object_load_by_drupal($field_settings['target_type'], $field->value)) {
      return $referenced_mapping->sfid();
    }
  }

  private function getConfigurationOptions($mapping) {
    $instances = Field::fieldInfo()->getBundleInstances(
      $mapping->get('drupal_entity_type'), 
      $mapping->get('drupal_bundle')
    );
    $options = array();
    foreach ($instances as $name => $instance) {
      if ($instance->getField()->get('type') != 'entity_reference') {
        continue;
      }
      $options[$name] = $instance->get('label');
    }
    return $options;
  }

}
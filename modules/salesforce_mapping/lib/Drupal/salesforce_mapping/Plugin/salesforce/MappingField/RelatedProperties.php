<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce\MappingField\RelatedProperties.
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
 *   id = "RelatedProperties",
 *   label = @Translation("Related Entity Properties")
 * )
 */
class RelatedProperties extends MappingFieldPluginBase {

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
      '#description' => $this->t('Select a property from the referenced field.<br />If more than one entity is referenced, the entity at delta zero will be used.<br />An entity reference field will be used to sync an identifier, e.g. Salesforce ID and Node ID.'),
    );
  }

  public function value(EntityInterface $entity) {
    list($field_name, $referenced_field_name) = explode(':', $this->config('drupal_field_value'), 2);
    // Since we're not setting hard restrictions around bundles/fields, we may
    // have a field that doesn't exist for the given bundle/entity. In that
    // case, calling get() on an entity with a non-existent field argument
    // causes an exception during entity save. Probably a bug, but I haven't
    // found it in the issue queue. So, just check first to make sure the field
    // exists.
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
    try {
      $referenced_entity = $this->entityManager
        ->getStorageController($field_settings['target_type'])
        ->load($field->value);
    }
    catch (Exception $e) {
      // @todo something about this exception
      return;
    }

    // Again, try to avoid some complicated fatal further downstream.
    $referenced_instances = $this->entityManager->getFieldDefinitions(
      $referenced_entity->entityType(),
      $referenced_entity->bundle()
    );
    if (empty($referenced_instances[$referenced_field_name])) {
      return;
    }
    return $referenced_entity->get($referenced_field_name)->value;
  }

  private function getConfigurationOptions($mapping) {
    // @todo: cache this. looping over every instance on every bundle and entity type could get expensive.
    $instances = Field::fieldInfo()->getBundleInstances(
      $mapping->get('drupal_entity_type'), 
      $mapping->get('drupal_bundle')
    );
    $options = array();
    foreach ($instances as $instance) {
      if ($instance->getField()->get('type') != 'entity_reference') {
        continue;
      }
      $field = $instance->getField();

      // We must have an entity type.
      if (!$field->getFieldSetting('target_type')) {
        continue;
      }

      $entity_type = $field->getFieldSetting('target_type');
      $properties = array();
      $settings = $instance->getFieldSettings();
      if ($settings['handler'] == 'default') {
        // If no target bundles, the entity type probably doesn't support them.
        // Fudge the settings array.
        if (empty($settings['handler_settings']['target_bundles'])) {
          $settings['handler_settings']['target_bundles'] = array(NULL);
        }
        // @todo: expose all fields for all target bundles. Figure out how to indicate in UI that it's possible to mis-configure this.
        foreach ($settings['handler_settings']['target_bundles'] as $bundle) {
          $properties += $this->entityManager->getFieldDefinitions(
            $entity_type,
            $bundle
          );
        }
      }
      else {
      // @todo: right now we only support simple reference fields. Work out support for views references by somehow gathering bundles from view settings.
        $properties = $this->entityManager
          ->getFieldDefinitions($field->getFieldSetting('target_type'));
      }
      foreach ($properties as $key => $property) {
        $options[$instance->label][$field->name.':'.$key] = $property['label'];
      }
    }
    return $options;
  }

}
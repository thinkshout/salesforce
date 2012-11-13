<?php

/**
 * @file
 * Data and callbacks for fieldmap types.
 */

/**
 * Implements hook_salesforce_mapping_fieldmap_type().
 */
function salesforce_mapping_salesforce_mapping_fieldmap_type() {
  $types = array(
    'property' => array(
      'label' => t('Properties'),
      'field_type' => 'select',
      'description' => t('Select a Drupal field or property to map to a Salesforce field.'),
      'options_callback' => 'salesforce_mapping_property_fieldmap_options',
      'value_callback' => 'salesforce_mapping_property_fieldmap_value',
    ),
    'token' => array(
      'label' => t('Token'),
      'description' => t('Enter a token to map a Salesforce field..'),
      'field_type' => 'textfield',
      'value_callback' => 'salesforce_mapping_token_fieldmap_value',
    ),
    'constant' => array(
      'label' => t('Constant'),
      'description' => t('Enter a constant value to map to a Salesforce field.'),
      'field_type' => 'textfield',
      'value_callback' => 'salesforce_mapping_constant_fieldmap_value',
    )
  );

  return $types;
}

/**
 * Return all possible Drupal entity field mappings.
 *
 * @param $entity_type
 * @param $bundle
 *
 * @return array
 */
function salesforce_mapping_property_fieldmap_options($entity_type) {
  $options = array('' => t('-- Select --'));

  $properties = entity_get_all_property_info($entity_type);
  foreach ($properties as $key => $property) {
    if (isset($property['field']) && $property['field'] && !empty($property['property info'])) {
      foreach ($property['property info'] as $sub_key => $sub_prop) {
        $options[$property['label']][$key . ':' . $sub_key] = $sub_prop['label'];
      }
    }
    else {
      $options[$key] = $property['label'];
    }
  }

  return $options;
}

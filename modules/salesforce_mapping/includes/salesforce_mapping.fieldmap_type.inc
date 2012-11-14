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
      'description' => t('Select a Drupal field or property to map to a Salesforce field. Related are left out and should be handled using another fieldmap type like tokens.'),
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
    ),
    'related_entity' => array(
      'label' => t('Related entities'),
      'field_type' => 'select',
      'description' => t('If a existing connection is found with the selected entity, the linked identifiers will be used..'),
      'options_callback' => 'salesforce_mapping_related_entity_fieldmap_options',
      'value_callback' => 'salesforce_mapping_related_entity_fieldmap_value',
    ),
  );

  return $types;
}

/**
 * Return all possible Drupal properties for a given entity type.
 *
 * @param $entity_type
 * @param $include_entities
 *
 * @return array
 */
function salesforce_mapping_property_fieldmap_options($entity_type, $include_entities = FALSE) {
  $options = array('' => t('-- Select --'));

  $properties = entity_get_all_property_info($entity_type);
  foreach ($properties as $key => $property) {
    $type = entity_property_extract_innermost_type($property['type']);
    $is_entity = (bool)entity_get_info($type);
    // Leave entities out of this.
    if (($is_entity && $include_entities) || (!$is_entity && !$include_entities)) {
      if (isset($property['field']) && $property['field'] && !empty($property['property info'])) {
        foreach ($property['property info'] as $sub_key => $sub_prop) {
          $options[$property['label']][$key . ':' . $sub_key] = $sub_prop['label'];
        }
      }
      else {
        $options[$key] = $property['label'];
      }
    }
  }

  return $options;
}

/**
 * Return all possible Drupal properties of type entity for a given entity type.
 *
 * @param $entity_type
 *
 * @return array
 */
function salesforce_mapping_related_entity_fieldmap_options($entity_type) {
  return salesforce_mapping_property_fieldmap_options($entity_type, TRUE);
}

/**
 * Value callback for related entity fieldmap type.
 *
 * @param $fieldmap
 * @param $entity_wrapper
 *
 * @return mixed
 */
function salesforce_mapping_related_entity_fieldmap_value($fieldmap, $entity_wrapper) {
  $value = NULL;
  $related_wrapper = $entity_wrapper->$fieldmap['drupal_field']['fieldmap_value']->get(0);
  $mapped_objects = salesforce_mapping_object_load_by_drupal(
    $related_wrapper->type(),
    $related_wrapper->getIdentifier()
  );
  if ($mapped_objects) {
    $value = $mapped_objects->salesforce_id;
  }
  return $value;
}

/**
 * Value callback for property fieldmap type.
 *
 * @param $fieldmap
 * @param $entity_wrapper
 *
 * @return mixed
 */
function salesforce_mapping_property_fieldmap_value($fieldmap, $entity_wrapper) {
  $drupal_fields_array = explode(':', $fieldmap['drupal_field']['fieldmap_value']);
  $parent = $entity_wrapper;
  foreach ($drupal_fields_array as $drupal_field) {
    $child_wrapper = $parent->{$drupal_field};
    $parent = $child_wrapper;
  }
  $value = $child_wrapper->value();

  // TODO: Handle entities that are linked to other SF Objects.
  if (is_array($value)) {
    $value = $value[0];
  }

  // Field type specific handling
  if ($child_wrapper->type() == 'date') {
    // Drupal provides a timestamp, Salesforce wants ISO 8601 formatted date/time,
    $value = format_date($value, 'custom', 'c');
  }

  return $value;
}

/**
 * Value callback for token fieldmap type.
 *
 * @param $fieldmap
 * @param $entity_wrapper
 *
 * @return mixed
 */
function salesforce_mapping_token_fieldmap_value($fieldmap, $entity_wrapper) {
  $token = $fieldmap['drupal_field']['fieldmap_value'];
  $value = token_replace($token, array(
    $entity_wrapper->type() => $entity_wrapper->value()
  ));
  return $value;
}

/**
 * Value callback for constant fieldmap type.
 *
 * @param $fieldmap
 * @param $entity_wrapper
 *
 * @return mixed
 */
function salesforce_mapping_constant_fieldmap_value($fieldmap, $entity_wrapper) {
  return $fieldmap['drupal_field']['fieldmap_value'];
}

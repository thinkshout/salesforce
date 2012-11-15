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
      'push_value_callback' => 'salesforce_mapping_property_fieldmap_value',
      'pull_value_callback' => 'salesforce_mapping_property_fieldmap_pull_value',
      'validation_callback' => 'salesforce_mapping_property_validation',
    ),
    'token' => array(
      'label' => t('Token'),
      'description' => t('Enter a token to map a Salesforce field..'),
      'field_type' => 'textfield',
      'push_value_callback' => 'salesforce_mapping_token_fieldmap_value',
    ),
    'constant' => array(
      'label' => t('Constant'),
      'description' => t('Enter a constant value to map to a Salesforce field.'),
      'field_type' => 'textfield',
      'push_value_callback' => 'salesforce_mapping_constant_fieldmap_value',
    ),
    'related_entity' => array(
      'label' => t('Related entities'),
      'field_type' => 'select',
      'description' => t('If a existing connection is found with the selected entity, the linked identifiers will be used..'),
      'options_callback' => 'salesforce_mapping_related_entity_fieldmap_options',
      'push_value_callback' => 'salesforce_mapping_related_entity_fieldmap_value',
      'pull_value_callback' => 'salesforce_mapping_related_entity_fieldmap_pull_value',
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
  if (property_exists($entity_wrapper->$fieldmap['drupal_field']['fieldmap_value'], 'isEntityList')) {
    $related_wrapper = $entity_wrapper->$fieldmap['drupal_field']['fieldmap_value']->get(0);
  }
  else {
    $related_wrapper = $entity_wrapper->$fieldmap['drupal_field']['fieldmap_value'];
  }
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
 * Pull value callback for property fieldmap type.
 */
function salesforce_mapping_related_entity_fieldmap_pull_value($entity_wrapper, $sf_object, $field_map) {
  $mapped_objects = salesforce_mapping_object_load_by_sfid($sf_object[$field_map['salesforce_field']]);
  $info = $entity_wrapper->info();
  if ($mapped_objects && !isset($info['relation_type'])) {
    $entity = entity_load($mapped_objects->entity_type, array($mapped_objects->entity_id));
    return $entity;
  }
  // Handle relations
  else {
    $type =  $info['parent']->type();
    $id =  $info['parent']->getIdentifier();
    $query = relation_query($type, $id);
    $query->propertyCondition('relation_type', $info['relation_type']);
    $results = $query->execute();
    if ($results) {
      $relations = relation_load_multiple(array_keys($results));
      $match = TRUE;
    }
    else {
      $match = FALSE;
    }

    if (isset($relations)) {
      foreach ($relations as $rid => $relation) {
        $match = TRUE;
        // Check if matches existing
        $current_endpoints = $relations[$rid]->endpoints;
        if (
          ($current_endpoints[LANGUAGE_NONE][0]['entity_type'] != $type) ||
          ($current_endpoints[LANGUAGE_NONE][0]['entity_id'] != $id) ||
          ($current_endpoints[LANGUAGE_NONE][1]['entity_type'] != $mapped_objects->entity_type) ||
          ($current_endpoints[LANGUAGE_NONE][1]['entity_id'] != $mapped_objects->entity_id)
        )
        {
          $match = FALSE;
        }
        // If same type and not a match delete relation
        if ($current_endpoints[LANGUAGE_NONE][1]['entity_type'] == $mapped_objects->entity_type && !$match) {
          relation_delete($rid);
        }
      }
    }
    // Create new relation and endpoints
    if ($mapped_objects && !$match) {
      $relation_type = $info['relation_type'];
      $endpoints = array(
        array('entity_type' => $type, 'entity_id' => $id),
        array('entity_type' => $mapped_objects->entity_type, 'entity_id' => $mapped_objects->entity_id),
      );
      $relation_new = relation_create($relation_type, $endpoints);
      relation_save($relation_new);
    }

  }
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
 * Pull value callback for property fieldmap type.
 */
function salesforce_mapping_property_fieldmap_pull_value($entity_wrapper, $sf_object, $field_map) {
  $value = $sf_object[$field_map['salesforce_field']];
  if ($entity_wrapper->type() == 'date' && isset($value))  {
    $value = strtotime($value);
  }
  return $value;
}

/*
 * Property field validation callback
 */
function salesforce_mapping_property_validation($drupal_field, $sf_field, $direction, $key) {
  $valid = FALSE;
  $length_valid = TRUE;

  // todo: add length validation

  // Type validation
  /*
   * Type validation array
   * direction[array Drupal type][array Salesforce type]
   */
  $verify_array = array(
    'sync' => array(
      'string' => array('text'),
      'int' => array('integer'),
      'id' => array('integer'),
      'datetime' => array('date'),
      'email' => array('text'),
    ),
    'drupal_sf' => array(
      'string' => array('text'),
      'int' => array('integer'),
      'id' => array('integer'),
      'datetime' => array('date'),
      'email' => array('text'),
    ),
    'sf_drupal' => array(
      'string' => array('text', 'int'),
      'int' => array('integer'),
      'id' => array('integer'),
      'datetime' => array('date'),
      'email' => array('text'),
    ),
  );

  $drupal_type = $drupal_field['type'];
  $sf_type_array = isset($verify_array[$direction][$sf_field['type']]) ? $verify_array[$direction][$sf_field['type']] : array();
  if (empty($sf_type_array)) {
    // If type is not found in validation array, assume valid
    return TRUE;
  }

  $type_valid = (array_search($drupal_type, $sf_type_array) === FALSE) ? FALSE : TRUE;
  $valid = $type_valid && $length_valid;
  if (!$valid) {
    form_set_error(
      "salesforce_field_mappings][{$key}][salesforce_field",
      t('Drupal field %drupal_label is of type %drupal_type, Salesforce field %sf_label is of type %sf_type and cannot be mapped in the %dir direction',
      array(
        '%drupal_label' => $drupal_field['label'],
        '%drupal_type' => $drupal_field['type'],
        '%sf_label' => $sf_field['label'],
        '%sf_type' => $sf_field['type'],
        '%dir' =>$direction,
      )
    ));
  }
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

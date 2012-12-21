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
      'push_value_callback' => 'salesforce_mapping_property_fieldmap_push_value',
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
      'description' => t('If an existing connection is found with the selected entity, the linked identifiers will be used.'),
      'options_callback' => 'salesforce_mapping_related_entity_fieldmap_options',
      'push_value_callback' => 'salesforce_mapping_related_entity_fieldmap_push_value',
      'pull_value_callback' => 'salesforce_mapping_related_entity_fieldmap_pull_value',
    ),
  );

  if (module_exists('relation_endpoint')) {
    $types['relation_endpoints'] = array(
      'label' => t('Relation endpoints'),
      'field_type' => 'select',
      'description' => t('Allows multiple Salesforce fields to be selected as endpoint values.'),
      'options_callback' => 'salesforce_mapping_relation_endpoints_fieldmap_options',
      'pull_value_callback' => 'salesforce_mapping_relation_endpoints_fieldmap_pull_value',
      'salesforce_multiple_fields' => TRUE,
    );
  }

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
    $type = isset($property['type']) ? entity_property_extract_innermost_type($property['type']) : 'text';
    $is_entity = ($type == 'entity') || (bool) entity_get_info($type);
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
function salesforce_mapping_related_entity_fieldmap_push_value($fieldmap, $entity_wrapper) {
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
  $mapped_objects = salesforce_mapping_object_load_by_sfid($sf_object[$field_map['salesforce_field']['name']]);
  $info = $entity_wrapper->info();
  if ($mapped_objects && !isset($info['relation_type'])) {
    $entity = entity_load_single($mapped_objects->entity_type, $mapped_objects->entity_id);
    return entity_metadata_wrapper($mapped_objects->entity_type, $entity);
  }
  // Handle relations
  else {
    $type = $info['parent']->type();
    $id = $info['parent']->getIdentifier();
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
        ) {
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
        array(
          'entity_type' => $mapped_objects->entity_type,
          'entity_id' => $mapped_objects->entity_id
        ),
      );
      $relation_new = relation_create($relation_type, $endpoints);
      relation_save($relation_new);
    }
  }

  return NULL;
}

/**
 * Return relation endpoint property as an option.
 *
 * @param $entity_type
 *
 * @return array
 */
function salesforce_mapping_relation_endpoints_fieldmap_options($entity_type) {
  $properties = salesforce_mapping_property_fieldmap_options($entity_type);
  if (isset($properties['endpoints'])) {
    return array('endpoints' => $properties['endpoints']);
  }
  else {
    return FALSE;
  }
}

/**
 * Pull value callback for relation endpoint fieldmap type.
 */
function salesforce_mapping_relation_endpoints_fieldmap_pull_value($entity_wrapper, $sf_object, $field_map) {

  $values = array();

  foreach ($field_map['salesforce_field'] as $sf_field) {
    $mapped_object = salesforce_mapping_object_load_by_sfid($sf_object[$sf_field['name']]);
    $entity = entity_load_single($mapped_object->entity_type, $mapped_object->entity_id);
    $values[] = entity_metadata_wrapper($mapped_object->entity_type, $entity);
  }

  return $values;
}


/**
 * Value callback for property fieldmap type.
 *
 * @param $fieldmap
 * @param $entity_wrapper
 *
 * @return mixed
 */
function salesforce_mapping_property_fieldmap_push_value($fieldmap, $entity_wrapper) {
  $drupal_fields_array = explode(':', $fieldmap['drupal_field']['fieldmap_value']);
  $parent = $entity_wrapper;
  foreach ($drupal_fields_array as $drupal_field) {
    if ($parent instanceof EntityListWrapper) {
      // First list<> types, get the property from the first item
      $child_wrapper = $parent->get(0)->{$drupal_field};
    }
    else {
      $child_wrapper = $parent->{$drupal_field};
    }
    $parent = $child_wrapper;
  }
  $value = $child_wrapper->value();

  // Always the take the first value when cardinality is > 0.
  if (is_array($value) && isset($value[0])) {
    $value = $value[0];
  }

  // Field type specific handling
  if ($child_wrapper->type() == 'date' && isset($value) && ($fieldmap['salesforce_field']['type'] == 'date' || $fieldmap['salesforce_field']['type'] == 'dateTime')) {
    // Drupal provides a timestamp, Salesforce wants ISO 8601 formatted date/time,
    $value = format_date($value, 'custom', 'c');
  }

  return $value;
}

/**
 * Pull value callback for property fieldmap type.
 */
function salesforce_mapping_property_fieldmap_pull_value($entity_wrapper, $sf_object, $field_map) {
  $value = $sf_object[$field_map['salesforce_field']['name']];

  switch ($entity_wrapper->type()) {
    case 'date':
      if (isset($value) && ($field_map['salesforce_field']['type'] == 'date' || $field_map['salesforce_field']['type'] == 'dateTime')) {
        $value = strtotime($value);
      }
      break;
    case 'boolean':
      if (isset($value)) {
        $value = (int) $value;
      }
      break;
    case 'text':
      $value = (string) $value;
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
   * direction[array Salesforce type][array Drupal type]
   */
  $verify_array = array(
    'sync' => array(
      'string' => array('text', 'list<text>'),
      'int' => array('integer'),
      'id' => array('integer'),
      'datetime' => array('date'),
      'email' => array('text'),
    ),
    'drupal_sf' => array(
      'string' => array('text', 'list<text>'),
      'int' => array('integer'),
      'id' => array('integer'),
      'datetime' => array('date'),
      'email' => array('text'),
    ),
    'sf_drupal' => array(
      'string' => array('text', 'int', 'list<text>'),
      'int' => array('integer'),
      'id' => array('integer'),
      'datetime' => array('date'),
      'email' => array('text'),
    ),
  );

  $drupal_type = isset($drupal_field['type']) ? $drupal_field['type'] : 'text';
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
          '%dir' => $direction,
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

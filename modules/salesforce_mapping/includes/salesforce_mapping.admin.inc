<?php

/**
 * Return a form for a salesforce mapping entity.
 *
 * @param $form
 * @param $form_state
 * @param null $mapping
 *
 * @return mixed
 */
function salesforce_mapping_form($form, &$form_state, SalesforceMapping $mapping = NULL, $op = 'edit') {
  if ($op == 'clone') {
    $mapping->label .= ' (cloned)';
    $mapping->name = '';
  }
  $form['#id'] = 'salesforce_mapping_form';
  $form_state['salesforce_mapping'] = $mapping;

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => isset($mapping) ? $mapping->label : '',
    '#description' => t('The human-readable name of this field mapping.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'machine_name',
    '#description' => t(''),
    '#default_value' => isset($mapping) ? $mapping->name : '',
    '#disabled' => !empty($mapping->name),
    '#machine_name' => array(
      'exists' => 'salesforce_mapping_load',
      'source' => array('label')
     ),
  );

  $form['drupal_entity'] = array(
    '#title' => t('Drupal entity'),
    '#type' => 'fieldset',
  );

  $form['drupal_entity']['drupal_entity_type'] = array(
    '#title' => t('Drupal Entity Type'),
    '#type' => 'select',
    '#description' => t('Select a Drupal entity type to map to a Salesforce object.'),
    '#options' => salesforce_mapping_drupal_entities(),
    '#default_value' => isset($mapping) ? $mapping->drupal_entity_type : 0,
    '#required' => TRUE,
    '#ajax' => array(
      'callback' => 'salesforce_mapping_form_callback',
      'wrapper' => 'salesforce_mapping_form'
    )
  );

  $drupal_entity_type = &$form_state['values']['drupal_entity_type'];
  if (!$drupal_entity_type && isset($mapping)) {
    $drupal_entity_type = $mapping->drupal_entity_type;
  }
  if ($drupal_entity_type) {
    $bundles = array('' => t('-- Select --'));
    $info  = entity_get_info($drupal_entity_type);
    foreach ($info['bundles'] as $key => $bundle) {
      $bundles[$key] = $bundle['label'];
    }
    $form['drupal_entity']['drupal_bundle'] = array(
      '#title' => t('Drupal Entity Bundle'),
      '#type' => 'select',
      '#required' => TRUE,
      '#description' => t('Select a Drupal entity bundle to map to a Salesforce object.'),
      '#options' => $bundles,
      '#default_value' => isset($mapping) ? $mapping->drupal_bundle : 0,
      '#ajax' => array(
        'callback' => 'salesforce_mapping_form_callback',
        'wrapper' => 'salesforce_mapping_form'
      )
    );
  }

  $form['salesforce_object'] = array(
    '#title' => t('Salesforce object'),
    '#type' => 'fieldset',
  );

  $sfapi = salesforce_get_api();
  $sfobjects = &$form_state['sf_objects'];
  if (!$sfobjects) {
    // Note that we're filtering SF object types to a reasonable subset.
    $sfobjects = $sfapi->objects(array('updateable' => TRUE, 'triggerable' => TRUE));
    $form_state['sf_objects'] = $sfobjects;
  }

  $sfobject_options = array('' => t('-- Select --'));
  foreach ($sfobjects as $object) {
    $sfobject_options[$object['name']] = $object['label'];
  }

  $form['salesforce_object']['salesforce_object_type'] = array(
    '#title' => t('Salesforce object'),
    '#type' => 'select',
    '#description' => t('Select a Salesforce object to map.'),
    '#default_value' => isset($mapping) ? $mapping->salesforce_object_type : 0,
    '#options' => $sfobject_options,
    '#ajax' => array(
      'callback' => 'salesforce_mapping_form_callback',
      'wrapper' => 'salesforce_mapping_form'
    ),
    '#required' => TRUE
  );

  $salesforce_object_type = &$form_state['values']['salesforce_object_type'];
  if (!$salesforce_object_type && isset($mapping)) {
    $salesforce_object_type = $mapping->salesforce_object_type;
  }
  if (!empty($salesforce_object_type)) {
    $sfobject = &$form_state['salesforce_mapping_object_' . $salesforce_object_type];
    if (!$sfobject) {
      $sfobject = $sfapi->objectDescribe($salesforce_object_type);
    }
    $type_count = count($sfobject['recordTypeInfos']);
    $sf_types = array('' => t('-- Select --'));
    foreach ($sfobject['recordTypeInfos'] as $type) {
      if ($type['name'] != 'Master') {
        $sf_types[$type['recordTypeId']] = $type['name'];
      }
    }
    // Check for custom record types
    $default = $type_count > 1? 0 : SALESFORCE_MAPPING_DEFAULT_RECORD_TYPE;
    $form['salesforce_object']['salesforce_record_type'] = array(
      '#title' => t('Salesforce record type'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce record type to map.'),
      '#default_value' => (isset($mapping->name) && $type_count > 1) ? $mapping->salesforce_record_type : $default,
      '#options' => $sf_types,
      '#required' => TRUE,
      '#access' => $type_count > 1,
    );

    $sf_fields = array('' => t('-- Select --'));
    foreach ($sfobject['fields'] as $sf_field) {
      $sf_fields[$sf_field['name']] = $sf_field['label'];
    }

    $mappings_count = isset($form_state['salesforce_mappings_count']) ? $form_state['salesforce_mappings_count'] : 0;
    if (!$mappings_count && isset($mapping)) {
      $mappings_count = count($mapping->field_mappings);
    }

    if (isset($form_state['triggering_element']) && $form_state['triggering_element']['#name'] == 'salesforce_add_field') {
      $mappings_count++;
      $form_state['salesforce_mappings_count'] = $mappings_count;
    }

    $drupal_entity_type = isset($form_state['values']['drupal_entity_type']) ? $form_state['values']['drupal_entity_type'] : $mapping->drupal_entity_type;
    $drupal_bundle = isset($form_state['values']['drupal_bundle']) ? $form_state['values']['drupal_bundle'] : $mapping->drupal_bundle;

    $drupal_fields = salesforce_mapping_drupal_fields($drupal_entity_type, $drupal_bundle);

    $form['salesforce_field_mappings_wrapper'] = array(
      '#title' => t('Salesforce field mappings wrapper'),
      '#type' => 'fieldset',
    );

    $form['salesforce_field_mappings_wrapper']['salesforce_field_mappings'] = array(
      '#theme' => 'salesforce_fieldmap_form_table',
      '#tree' => TRUE,
      '#attributes' => array('id' => 'salesforce_field_mappings')
    );

    $field_mapping_rows = $mappings_count;
    for ($delta = 0; $delta <= $field_mapping_rows; $delta++) {
      $delete = &$form_state['values']['salesforce_field_mappings'][$delta]['delete_field_mapping'];
      if ($delete) {
        $mappings_count--;
        unset($mapping->field_mappings[$delta]);
        $form_state['salesforce_mappings_count'] = $mappings_count;
        continue;
      }
      $form['salesforce_field_mappings_wrapper']['salesforce_field_mappings'][$delta] = array(
        'drupal_field' => array(
          '#type' => 'select',
          '#description' => t('Select a Drupal field or property to map to a Salesforce field.'),
          '#options' => $drupal_fields,
          '#default_value' => isset($mapping->field_mappings[$delta]) ? $mapping->field_mappings[$delta]['drupal_field'] : 0,
        ),
        'salesforce_field' => array(
          '#type' => 'select',
          '#description' => t('Select a Salesforce field to map.'),
          '#options' => $sf_fields,
          '#default_value' => isset($mapping->field_mappings[$delta]) ? $mapping->field_mappings[$delta]['salesforce_field'] : 0,
        ),
        'key' => array(
          '#type' => 'radio',
          '#name' => 'key',
          '#return_value' => $delta,
          '#tree' => FALSE,
          '#default_value' => (isset($mapping->field_mappings[$delta]['key']) && $mapping->field_mappings[$delta]['key']) ? $delta : FALSE,
        ),
        'direction' => array(
          '#type' => 'radios',
          '#options' => array(
            SALESFORCE_MAPPING_DIRECTION_DRUPAL_SF => t('Drupal to SF'),
            SALESFORCE_MAPPING_DIRECTION_SF_DRUPAL => t('SF to Drupal'),
            SALESFORCE_MAPPING_DIRECTION_SYNC => t('Sync'),
          ),
          '#required' => TRUE,
          '#default_value' => isset($mapping->field_mappings[$delta]) ? $mapping->field_mappings[$delta]['direction'] : SALESFORCE_MAPPING_DIRECTION_SYNC,
        ),
        'delete_field_mapping' => array(
          '#type' => 'checkbox',
          '#ajax' => array(
            'callback' => 'salesforce_mapping_form_field_callback',
            'wrapper' => 'salesforce_field_mappings'
          )
        ),
      );
    }

    $form['salesforce_field_mappings_wrapper']['ajax_warning'] = array (
      '#type' => 'container',
    );

    $form['salesforce_field_mappings_wrapper']['salesforce_add_field'] = array(
      '#value' => t('Add another field mapping'),
      '#name' => 'salesforce_add_field',
      '#type' => 'button',
      '#description' => t('Add one or more field to configure a mapping for.'),
      '#ajax' => array(
        'callback' => 'salesforce_mapping_form_field_callback',
        'wrapper' => 'salesforce_field_mappings'
      )
    );
  }

  $default_triggers = array();
  if (isset($mapping)) {
    foreach (salesforce_mapping_sync_trigger_options() as $key => $option) {
      if ($key & $mapping->sync_triggers) {
        $default_triggers[] = $key;
      }
    }
  }

  $form['sync_triggers'] = array(
    '#title' => t('Action triggers'),
    '#type' => 'checkboxes',
    '#description' => t('Select which actions on Drupal entities and Salesforce objects should trigger a synchronization. These settings are used by the salesforce_push and salesforce_pull modules respectively.'),
    '#default_value' => $default_triggers,
    '#options' => salesforce_mapping_sync_trigger_options(),
    '#required' => TRUE,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#value' => t('Save mapping'),
    '#type' => 'submit',
  );

  return $form;
}

/**
 * Return form options for available sync triggers.
 *
 * @return array
 */
function salesforce_mapping_sync_trigger_options() {
  return array(
    SALESFORCE_MAPPING_SYNC_DRUPAL_CREATE => t('Drupal entity create'),
    SALESFORCE_MAPPING_SYNC_DRUPAL_UPDATE => t('Drupal entity update'),
    SALESFORCE_MAPPING_SYNC_DRUPAL_DELETE => t('Drupal entity delete'),
    SALESFORCE_MAPPING_SYNC_SF_CREATE => t('Salesforce object create'),
    SALESFORCE_MAPPING_SYNC_SF_UPDATE => t('Salesforce object update'),
    SALESFORCE_MAPPING_SYNC_SF_DELETE => t('Salesforce object delete'),
  );
}

/**
 * Return all possible Drupal entity field mappings.
 *
 * @param $entity_type
 * @param $bundle
 *
 * @return array
 */
function salesforce_mapping_drupal_fields($entity_type, $bundle) {
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

/**
 * Return a list of Drupal entities that implement EntityAPIControllerInterface.
 *
 * @return array
 */
function salesforce_mapping_drupal_entities() {
  $types = array('' => t('-- Select --'));
  foreach (entity_entity_property_info() as $type => $value) {
    if ($info = entity_get_info($type)) {
      $types[$type] = $info['label'];
    }
  }

  return $types;
}

/**
 * Ajax callback for salesforce_mapping_form().
 */
function salesforce_mapping_form_callback($form, $form_state) {
  return $form;
}

/**
 * Ajax callback for deleting a field mapping row.
 */
function salesforce_mapping_form_field_callback($form, $form_state) {
  $ajax_warn = array (
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('messages', 'warning'),
    ),
    'message' => array(
      '#type' => 'markup',
      '#markup'=> 'Changes made in this list will not be saved until the form is submitted.'
    ),
  );
  return array(
    '#type' => 'ajax',
    '#commands' => array(
      ajax_command_replace("#salesforce_field_mappings", render($form['salesforce_field_mappings_wrapper']['salesforce_field_mappings'])),
      ajax_command_replace("#edit-ajax-warning", render($ajax_warn)),
    )
  );
}

/**
 * Themes the field associations on a fieldmap edit form into a table.
 */
function theme_salesforce_fieldmap_form_table($variables) {
  $elements = $variables['elements'];

  // Build the header array.
  $header = array(
    t('Drupal field'),
    t('Salesforce field'),
    t('key'),
    t('Direction'),
    t('Delete'),
  );

  // Build the rows array.
  $rows = array();
  foreach (element_children($elements) as $child_key) {
    $child = &$elements[$child_key];
    $rows[] = array(
      array(
        'data' => drupal_render($child['drupal_field'])
      ),
      array(
        'data' => drupal_render($child['salesforce_field'])
      ),
      array(
        'data' => drupal_render($child['key'])
      ),
      array(
       'data' => drupal_render($child['direction']),
      ),
      array(
       'data' => drupal_render($child['delete_field_mapping']),
      )
    );
  }

  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => $elements['#attributes']));
}

/**
 * Validate salesforce_mapping_form().
 *
 * @param $form
 * @param $form_state
 */
function salesforce_mapping_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $mapping = $form_state['salesforce_mapping'];

  $efq = new EntityFieldQuery();
  $efq
    ->entityCondition('entity_type', 'salesforce_mapping')
    ->propertyCondition('salesforce_object_type', $values['salesforce_object_type'])
    ->propertyCondition('salesforce_record_type', $values['salesforce_record_type'])
    ->propertyCondition('drupal_entity_type', $values['drupal_entity_type'])
    ->propertyCondition('drupal_bundle', $values['drupal_bundle'])
    ->count();

  // existing mapping, ensure not using any other unique combo
  if ($mapping) {
    $efq->propertyCondition('name', $mapping->name, '<>');
  }

  $count = $efq->execute();
  if ($count > 0) {
    form_set_error('drupal_entity_type', t('This Drupal bundle has already been mapped to a Salesforce object.'));
  }

  // If Salesforce Object Create is selected, ensure that at least one mapping is set to sync or SF to Drupal to prevent "empty" entities
  if ($values['sync_triggers'][SALESFORCE_MAPPING_SYNC_SF_CREATE] && isset($values['salesforce_field_mappings'])) {
    $valid_salesforce_create = FALSE;
    foreach ($values['salesforce_field_mappings'] as $mapping) {
      if ($mapping['direction'] ==  SALESFORCE_MAPPING_DIRECTION_SYNC || $mapping['direction'] == SALESFORCE_MAPPING_DIRECTION_SF_DRUPAL) {
        $valid_salesforce_create = TRUE;
        break;
      }
    }
    if (!$valid_salesforce_create) {
      form_set_error('mapping issue', t('One mapping must be set to "Sync" or "SF to Drupal" when "Salesforce object create" is selected'));
    }
  }

  // Validate field mappings.
  $info = entity_get_info($values['drupal_entity_type']);
  $properties = entity_get_all_property_info($values['drupal_entity_type']);
  $sf_fields = $form_state['salesforce_mapping_object_' . $values['salesforce_object_type']]['fields'];
  $key_delta = $values['key'];
  foreach ($values['salesforce_field_mappings'] as $key => $mapping) {
    if (!empty($mapping['drupal_field']) && !empty($mapping['salesforce_field'])) {
      $field_array = explode(':', $mapping['drupal_field']);
      $drupal_field = count($field_array) === 1 ? $properties[$field_array[0]] : $properties[$field_array[0]]['property info'][$field_array[1]];
      $sf_field = NULL;
      foreach ($sf_fields as $field) {
        if ($field['name'] == $mapping['salesforce_field']) {
          $sf_field = $field;
          if (!salesforce_mapping_field_validation($drupal_field, $sf_field, $mapping['direction'])) {
            form_set_error(
              "salesforce_field_mappings][{$key}][salesforce_field",
              t('Drupal field %drupal_label is of type %drupal_type, Salesforce field %sf_label is of type %sf_type and cannot be mapped in the %dir direction',
              array(
                '%drupal_label' => $drupal_field['label'],
                '%drupal_type' => $drupal_field['type'],
                '%sf_label' => $sf_field['label'],
                '%sf_type' => $sf_field['type'],
                '%dir' =>$mapping['direction'],
              )
            ));
          }
          break;
        }
      }

      // ensure valid SF external ID field
      if ($key_delta && $key === (int)$key_delta && !$sf_field['externalId']) {
        form_set_error('salesforce_field_mappings][0][salesforce_field', t('Salesforce field %name is not configured as an external id.',
          array(
            '%name' => $mapping['salesforce_field']
          )
        ));
      }
    }
  }
}

/*
 * Field validation
 */
function salesforce_mapping_field_validation($sf_field, $drupal_field, $direction) {
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

  $sf_type = $sf_field['type'];
  $drupal_type_array = isset($verify_array[$direction][$drupal_field['type']])?$verify_array[$direction][$drupal_field['type']]:array();
  if (empty($drupal_type_array)) {
    // If type is not found in validation array, assume valid
    return TRUE;
  }
  $type_valid = (array_search($sf_type, $drupal_type_array) === FALSE) ? FALSE : TRUE;
  $valid = $type_valid && $length_valid;
  return $valid;
}
/**
 * Submit handler for salesforce_mapping_form().
 *
 * @param $form
 * @param $form_state
 */
function salesforce_mapping_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $mappings = array();
  $key_delta = $values['key'];
  foreach ($values['salesforce_field_mappings'] as $key => $mapping) {
    if (!empty($mapping['drupal_field']) && !empty($mapping['salesforce_field'])) {
      $mappings[] = array(
        'drupal_field' => $mapping['drupal_field'],
        'salesforce_field' => $mapping['salesforce_field'],
        'key' => $key_delta && ($key === (int)$key_delta),
        'direction' => $mapping['direction']
      );
    }
  }

  $sync_values = array_filter($values['sync_triggers']);
  $sync_triggers = SALESFORCE_MAPPING_SYNC_OFF;
  foreach ($sync_values as $value) {
    $sync_triggers = $sync_triggers | $value;
  }

  if ($form_state['salesforce_mapping']->name) {
    $mapping = $form_state['salesforce_mapping'];
    $mapping->label = $values['label'];
    $mapping->salesforce_object_type = $values['salesforce_object_type'];
    $mapping->salesforce_record_type = $values['salesforce_record_type'];
    $mapping->drupal_entity_type = $values['drupal_entity_type'];
    $mapping->drupal_bundle = $values['drupal_bundle'];
    $mapping->description = '';
    $mapping->field_mappings = $mappings;
    $mapping->sync_triggers = $sync_triggers;
  }
  else {
    $mapping = entity_create('salesforce_mapping', array(
      'name' => $values['name'],
      'label' => $values['label'],
      'salesforce_object_type' => $values['salesforce_object_type'],
      'salesforce_record_type' => $values['salesforce_record_type'],
      'drupal_entity_type' => $values['drupal_entity_type'],
      'drupal_bundle' => $values['drupal_bundle'],
      'description' => '',
      'field_mappings' => $mappings,
      'sync_triggers' => $sync_triggers
    ));
  }

  $mapping->save();

  drupal_set_message(t('Salesforce field mapping saved.'));
  $form_state['redirect'] = 'admin/structure/salesforce/mappings';
}

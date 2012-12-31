<?php

define('SALESFORCE_MAPPING_NAME_LENGTH', 128);

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
    '#maxlength' => SALESFORCE_MAPPING_NAME_LENGTH,
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
    '#attributes' => array(
      'id' => array('edit-drupal-entity'),
    ),
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

  $drupal_entity_type = & $form_state['values']['drupal_entity_type'];
  if (!$drupal_entity_type && isset($mapping)) {
    $drupal_entity_type = $mapping->drupal_entity_type;
  }
  if ($drupal_entity_type) {
    $bundles = array('' => t('-- Select --'));
    $info = entity_get_info($drupal_entity_type);
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
    '#attributes' => array(
      'id' => array('edit-salesforce-object'),
    ),
  );

  $sfapi = salesforce_get_api();
  $sfobjects = & $form_state['sf_objects'];
  if (!$sfobjects) {
    // Note that we're filtering SF object types to a reasonable subset.
    $sfobjects = $sfapi->objects(array(
      'updateable' => TRUE,
      'triggerable' => TRUE
    ));
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

  $salesforce_object_type = & $form_state['values']['salesforce_object_type'];
  if (!$salesforce_object_type && isset($mapping)) {
    $salesforce_object_type = $mapping->salesforce_object_type;
  }
  if (!empty($salesforce_object_type)) {
    $sfobject = & $form_state['salesforce_mapping_object_' . $salesforce_object_type];
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
    $default = $type_count > 1 ? 0 : SALESFORCE_MAPPING_DEFAULT_RECORD_TYPE;
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

    if ((isset($form_state['triggering_element']) && $form_state['triggering_element']['#name'] == 'salesforce_add_field') ||
      !isset($mapping->field_mappings)
    ) {
      $mapping->field_mappings[] = array(
        'drupal_field' => '',
        'salesforce_field' => '',
        'key' => '',
        'direction' => SALESFORCE_MAPPING_DIRECTION_SYNC,
      );
    }

    $form['salesforce_field_mappings_wrapper'] = array(
      '#title' => t('Salesforce field mappings wrapper'),
      '#type' => 'fieldset',
    );

    $form['salesforce_field_mappings_wrapper']['salesforce_field_mappings'] = array(
      '#theme' => 'salesforce_fieldmap_form_table',
      '#tree' => TRUE,
      '#attributes' => array('id' => 'salesforce_field_mappings')
    );

    $drupal_type_options = array('' => t('-- Select --'));
    foreach (salesforce_mapping_get_fieldmap_types() as $key => $fieldmap_type) {
      $drupal_type_options[$key] = $fieldmap_type['label'];
    }

    $has_token_type = FALSE;
    foreach ($mapping->field_mappings as $delta => $value) {
      $delete = & $form_state['values']['salesforce_field_mappings'][$delta]['delete_field_mapping'];
      if ($delete) {
        unset($mapping->field_mappings[$delta]);
        continue;
      }

      $fieldmap_type_name = & $form_state['values']['salesforce_field_mappings'][$delta]['drupal_field']['fieldmap_type'];
      if (!$fieldmap_type_name && isset($mapping)) {
        $fieldmap_type_name = $mapping->field_mappings[$delta]['drupal_field']['fieldmap_type'];
      }

      if ($fieldmap_type_name == 'token') {
        $has_token_type = TRUE;
      }

      $fieldmap_value_element = array(
        '#prefix' => '<div id="drupal_fieldmap_value_wrapper' . $delta . '">',
        '#suffix' => '</div>',
      );
      if ($fieldmap_type_name) {
        $fieldmap_type = salesforce_mapping_get_fieldmap_types($fieldmap_type_name);
        $fieldmap_value_element += array(
          '#type' => $fieldmap_type['field_type'],
          '#description' => $fieldmap_type['description'],
          '#size' => !empty($fieldmap_type['description']) ? $fieldmap_type['description'] : 30,
          '#default_value' => isset($mapping) ? $mapping->field_mappings[$delta]['drupal_field']['fieldmap_value'] : NULL,
        );
        if (!empty($fieldmap_type['options_callback'])) {
          $fieldmap_value_element['#options'] = call_user_func($fieldmap_type['options_callback'], $form_state['values']['drupal_entity_type']);
        }
      }

      $sf_field_default = 0;
      if (isset($mapping->field_mappings[$delta])) {
        if (isset($fieldmap_type['salesforce_multiple_fields']) && $fieldmap_type['salesforce_multiple_fields']) {
          $sf_field_default = array();
          foreach ($mapping->field_mappings[$delta]['salesforce_field'] as $field) {
            $sf_field_default[] = $field['name'];
          }
        }
        else {
          $sf_field_default = $mapping->field_mappings[$delta]['salesforce_field']['name'];
        }
      }

      $form['salesforce_field_mappings_wrapper']['salesforce_field_mappings'][$delta] = array(
        'drupal_field' => array(
          'fieldmap_type' => array(
            '#type' => 'select',
            '#options' => $drupal_type_options,
            '#default_value' => isset($mapping) ? $mapping->field_mappings[$delta]['drupal_field']['fieldmap_type'] : 0,
            '#ajax' => array(
              'wrapper' => "salesforce_field_mappings",
              'callback' => 'salesforce_mapping_form_field_callback'
            )
          ),
          'fieldmap_value' => $fieldmap_value_element
        ),
        'salesforce_field' => array(
          '#type' => 'select',
          '#description' => t('Select a Salesforce field to map.'),
          '#multiple' => (isset($fieldmap_type['salesforce_multiple_fields']) && $fieldmap_type['salesforce_multiple_fields']) ? TRUE : FALSE,
          '#options' => $sf_fields,
          '#default_value' => $sf_field_default,
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

    $form['salesforce_field_mappings_wrapper']['ajax_warning'] = array(
      '#type' => 'container',
    );

    $form['salesforce_field_mappings_wrapper']['token_tree'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => array('edit-token-tree'),
      ),
    );

    if ($has_token_type) {
      $form['salesforce_field_mappings_wrapper']['token_tree']['tree'] = array(
        '#theme' => 'token_tree',
        '#token_types' => array($drupal_entity_type),
        '#global_types' => TRUE,
      );
    }

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

  $form['push_async'] = array(
    '#title' => t('Process asynchronously'),
    '#type' => 'checkbox',
    '#description' => t('If selected, push data will be queued for processing and synchronized when cron is run. This may increase site performance, but changes will not be reflected in real time.'),
    '#default_value' => isset($mapping) ? $mapping->push_async : 0
  );
  $form['push_batch'] = array(
    '#title' => t('Batch records'),
    '#type' => 'checkbox',
    '#description' => t('Push items will be processed in a single batch to the Salesforce API rather than one at a time. This may be preferable if API limits are of concern, although changes are not reflected immediately in Salesforce. Requires the salesforce_soap module and asynchronous processing to be enabled.'),
    '#default_value' => isset($mapping) ? $mapping->push_batch : 0,
    '#disabled' => module_exists('salesforce_soap'),
    '#states' => array(
      'enabled' => array(
        ':input[name="push_async"]' => array('checked' => TRUE),
      )
    )
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
 * Return a list of Drupal entities that implement EntityAPIControllerInterface.
 *
 * @return array
 */
function salesforce_mapping_drupal_entities() {
  module_load_include('inc', 'entity', 'entity.info');
  $types = array('' => t('-- Select --'));
  foreach (entity_entity_property_info() as $type => $value) {
    if ($info = entity_get_info($type)) {
      // Exclude Salesforce Mapping and Salesforce Object Mapping entities as
      // these will never sync with Salesforce.
      if (in_array($type, array('salesforce_mapping', 'salesforce_mapping_object'))) {
        continue;
      }
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
  $errors = array('#theme' => 'status_messages');
  $ajax_warn = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('messages', 'warning'),
    ),
    'message' => array(
      '#type' => 'markup',
      '#markup' => 'Changes made in this list will not be saved until the form is submitted.'
    ),
  );
  $commands = array(
    ajax_command_replace("#salesforce_field_mappings", render($form['salesforce_field_mappings_wrapper']['salesforce_field_mappings'])),
    ajax_command_replace("#edit-token-tree", render($form['salesforce_field_mappings_wrapper']['token_tree'])),
    ajax_command_replace("#edit-ajax-warning", render($ajax_warn)),
    // Set message.
    ajax_command_remove('.messages'),
    ajax_command_after('#page-title', render($errors)),
    ajax_command_replace('#edit-salesforce-object', render($form['salesforce_object'])),
    ajax_command_replace('#edit-drupal-entity', render($form['drupal_entity'])),
  );
  return array(
    '#type' => 'ajax',
    '#commands' => $commands
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
    $child = & $elements[$child_key];
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

  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => $elements['#attributes']
  ));
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

  // validate label and name length
  if (strlen($values['label']) > SALESFORCE_MAPPING_NAME_LENGTH) {
    form_set_error('label', t('Label must not exceed @max characters.', array('@max' => SALESFORCE_MAPPING_NAME_LENGTH)));
  }
  if (strlen($values['name']) > SALESFORCE_MAPPING_NAME_LENGTH) {
    form_set_error('name', t('Name must not exceed @max characters.', array('@max' => SALESFORCE_MAPPING_NAME_LENGTH)));
  }

  $efq = new EntityFieldQuery();
  $efq
    ->entityCondition('entity_type', 'salesforce_mapping')
    ->propertyCondition('salesforce_object_type', $values['salesforce_object_type'])
    ->propertyCondition('salesforce_record_type', $values['salesforce_record_type'])
    ->propertyCondition('drupal_entity_type', $values['drupal_entity_type'])
    ->propertyCondition('drupal_bundle', $values['drupal_bundle'])
    ->count();

  // Existing mapping, ensure not using any other unique combo.
  if ($mapping->name) {
    $efq->propertyCondition('name', $mapping->name, '<>');
  }

  $count = $efq->execute();
  if ($count > 0) {
    form_set_error('drupal_bundle', t('This Drupal bundle has already been mapped to a Salesforce object.'));
  }

  // If Salesforce Object Create is selected, ensure that at least one mapping is set to sync or SF to Drupal to prevent "empty" entities
  if ($values['sync_triggers'][SALESFORCE_MAPPING_SYNC_SF_CREATE] && isset($values['salesforce_field_mappings'])) {
    $valid_salesforce_create = FALSE;
    foreach ($values['salesforce_field_mappings'] as $mapping) {
      if ($mapping['direction'] == SALESFORCE_MAPPING_DIRECTION_SYNC || $mapping['direction'] == SALESFORCE_MAPPING_DIRECTION_SF_DRUPAL) {
        $valid_salesforce_create = TRUE;
        break;
      }
    }
    if (!$valid_salesforce_create) {
      form_set_error('mapping issue', t('One mapping must be set to "Sync" or "SF to Drupal" when "Salesforce object create" is selected'));
    }
  }

  if (isset($values['salesforce_field_mappings'])) {
    // Validate field mappings.
    $info = entity_get_info($values['drupal_entity_type']);
    $properties = entity_get_all_property_info($values['drupal_entity_type']);
    $sf_fields = $form_state['salesforce_mapping_object_' . $values['salesforce_object_type']]['fields'];
    $key_delta = $values['key'];

    foreach ($values['salesforce_field_mappings'] as $key => $mapping) {
      $drupal_field_type = $mapping['drupal_field']['fieldmap_type'];
      $field_map = salesforce_mapping_get_fieldmap_types($drupal_field_type);
      if (!empty($mapping['drupal_field']) && !empty($mapping['salesforce_field']) && isset($field_map['validation_callback'])) {
        if (isset($field_map['salesforce_multiple_fields']) && $field_map['salesforce_multiple_fields']) {
          $sf_field = array();
          foreach ($mapping['salesforce_field'] as $field) {
            $sf_field[] = reset(array_values(array_filter($sf_fields, salesforce_field_filter($field))));
          }
        }
        else {
          $filtered = array_filter($sf_fields, salesforce_field_filter($mapping['salesforce_field']));
          $sf_field = reset($filtered);
        }

        $field_array = explode(':', $mapping['drupal_field']['fieldmap_value']);
        $drupal_field = count($field_array) === 1 ? $properties[$field_array[0]] : $properties[$field_array[0]]['property info'][$field_array[1]];
        if (isset($field_map['salesforce_multiple_fields']) && $field_map['salesforce_multiple_fields']) {
          foreach ($sf_field as $field) {
            call_user_func($field_map['validation_callback'], $drupal_field, $field, $mapping['direction'], $key);
          }
        }
        else {
          call_user_func($field_map['validation_callback'], $drupal_field, $sf_field, $mapping['direction'], $key);
        }
        // ensure valid SF external ID field
        if ($key_delta && $key === (int) $key_delta && !$sf_field['externalId']) {
          form_set_error('salesforce_field_mappings][0][salesforce_field', t('Salesforce field %name is not configured as an external id.',
            array(
              '%name' => $mapping['salesforce_field']['name']
            )
          ));
        }
      }
    }
  }
}

/*
 * Closure used sf_field filter
 */
function salesforce_field_filter($sf_mapping_field) {
  return function ($sf_field) use ($sf_mapping_field) {
    $pass = FALSE;
    if ($sf_mapping_field == $sf_field['name']) {
      $pass = TRUE;
    }
    return $pass;
  };
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
  if (isset($values['key'])) {
    $key_delta = $values['key'];
    $sf_fields = $form_state['salesforce_mapping_object_' . $values['salesforce_object_type']]['fields'];
    foreach ($values['salesforce_field_mappings'] as $key => $mapping) {
      $drupal_field_type = $mapping['drupal_field']['fieldmap_type'];
      $field_map = salesforce_mapping_get_fieldmap_types($drupal_field_type);
      if (!empty($mapping['drupal_field']) && !empty($mapping['salesforce_field'])) {
        if (isset($field_map['salesforce_multiple_fields']) && $field_map['salesforce_multiple_fields']) {
          $sf_field = array();
          foreach ($mapping['salesforce_field'] as $field) {
            $sf_field[] = reset(array_filter($sf_fields, salesforce_field_filter($field)));
          }
        }
        else {
          $filtered = array_filter($sf_fields, salesforce_field_filter($mapping['salesforce_field']));
          $sf_field = reset($filtered);
        }

        $mappings[] = array(
          'drupal_field' => $mapping['drupal_field'],
          'salesforce_field' => $sf_field,
          'key' => $key_delta && ($key === (int) $key_delta),
          'direction' => $mapping['direction']
        );
      }
    }
  }

  $sync_values = array_filter($values['sync_triggers']);
  $sync_triggers = SALESFORCE_MAPPING_SYNC_OFF;
  foreach ($sync_values as $value) {
    $sync_triggers = $sync_triggers | $value;
  }

  $params = array(
    'label' => $values['label'],
    'salesforce_object_type' => $values['salesforce_object_type'],
    'salesforce_record_type' => $values['salesforce_record_type'],
    'drupal_entity_type' => $values['drupal_entity_type'],
    'drupal_bundle' => $values['drupal_bundle'],
    'description' => '',
    'field_mappings' => $mappings,
    'sync_triggers' => $sync_triggers,
    'push_async' => $values['push_async'],
    'push_batch' => $values['push_batch'],
  );

  if ($form_state['salesforce_mapping']->name) {
    $mapping = $form_state['salesforce_mapping'];
    foreach ($params as $key => $value) {
      $mapping->{$key} = $value;
    }
  }
  else {
    $params['name'] = $values['name'];
    $mapping = entity_create('salesforce_mapping', $params);
  }

  $mapping->save();

  drupal_set_message(t('Salesforce field mapping saved.'));
  $form_state['redirect'] = 'admin/structure/salesforce/mappings';
}

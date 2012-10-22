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
function salesforce_mapping_form($form, &$form_state, SalesforceMapping $mapping = NULL) {
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

  $types = array('' => t('-- Select --'));
  foreach (entity_get_info() as $type => $info) {
    $types[$type] = $info['label'];
  }
  $form['drupal_entity']['drupal_entity_type'] = array(
    '#title' => t('Drupal Entity Type'),
    '#type' => 'select',
    '#description' => t('Select a Drupal entity type to map to a Salesforce object.'),
    '#options' => $types,
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
    $sfobjects = $sfapi->objects();
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
    $sf_types = array('' => t('-- Select --'));
    foreach ($sfobject['recordTypeInfos'] as $type) {
      $sf_types[$type['recordTypeId']] = $type['name'];
    }
    $form['salesforce_object']['salesforce_record_type'] = array(
      '#title' => t('Salesforce record type'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce record type to map.'),
      '#default_value' => isset($mapping) ? $mapping->salesforce_record_type : 0,
      '#options' => $sf_types,
      '#required' => TRUE,
    );

    $sf_fields = array('' => t('-- Select --'));
    foreach ($sfobject['fields'] as $sf_field) {
      $sf_fields[$sf_field['name']] = $sf_field['label'];
    }

    $form['salesforce_field_mappings'] = array(
      '#theme' => 'salesforce_fieldmap_form_table',
      '#tree' => TRUE
    );

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

    for ($delta = 0; $delta <= $mappings_count; $delta++) {
      $form['salesforce_field_mappings'][$delta] = array(
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
        'remove' => array()
      );
    }

    $form['salesforce_add_field'] = array(
      '#value' => t('Add another field mapping'),
      '#name' => 'salesforce_add_field',
      '#type' => 'button',
      '#description' => t('Add one or more field to configure a mapping for.'),
      '#limit_validation_errors' => array(),
      '#ajax' => array(
        'callback' => 'salesforce_mapping_form_callback',
        'wrapper' => 'salesforce_mapping_form'
      )
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#value' => t('Save mapping'),
    '#type' => 'submit',
  );

  return $form;
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
  $fields = field_info_instances($entity_type, $bundle);
  foreach ($fields as $field_name => $field_info) {
    $options['Fields'][$field_name] = $field_info['label'];
  }

  $properties = entity_get_all_property_info($entity_type);
  foreach ($properties as $key => $property) {
    $options['Properties'][$key] = $property['label'];
  }

  return $options;
}

/**
 * Ajax callback for salesforce_mapping_form().
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function salesforce_mapping_form_callback($form, $form_state) {
  // @todo: This is lazy, we should only return appropriate child elements.
  return $form;
}


/**
 * Themes the field associations on a fieldmap edit form into a table.
 */
function theme_salesforce_fieldmap_form_table($variables) {
  $elements = $variables['elements'];

  // Build the header array.
  $header = array(
    t('Salesforce field'),
    t('Drupal field'),
    t('Settings'),
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
       'data' => $child['remove'],
      )
    );
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
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
  foreach ($values['salesforce_field_mappings'] as $mapping) {
    if (!empty($mapping['drupal_field']) && !empty($mapping['salesforce_field'])) {
      $mappings[] = array(
        'drupal_field' => $mapping['drupal_field'],
        'salesforce_field' => $mapping['salesforce_field']
      );
    }
  }

  $mapping = $form_state['salesforce_mapping'];
  if ($mapping) {
    $mapping->label = $values['label'];
    $mapping->salesforce_object_type = $values['salesforce_object_type'];
    $mapping->salesforce_record_type = $values['salesforce_record_type'];
    $mapping->drupal_entity_type = $values['drupal_entity_type'];
    $mapping->drupal_bundle = $values['drupal_bundle'];
    $mapping->description = '';
    $mapping->field_mappings = $mappings;
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
      'field_mappings' => $mappings
    ));
  }

  $mapping->save();

  drupal_set_message(t('Salesforce field mapping created.'));
  $form_state['redirect'] = 'admin/config/services/salesforce/mappings';
}

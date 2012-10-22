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
function salesforce_mapping_form($form, &$form_state, $mapping = NULL) {
  $form['#id'] = 'salesforce_mapping_form';

  $form['drupal_entity'] = array(
    '#title' => t('Drupal entity'),
    '#type' => 'fieldset',
  );

  $types = array('' => t('-- Select --'));
  foreach (entity_get_info() as $type => $info) {
    $types[$type] = $info['label'];
  }
  $form['drupal_entity']['drupal_type'] = array(
    '#title' => t('Drupal Entity Type'),
    '#type' => 'select',
    '#description' => t('Select a Drupal entity type to map to a Salesforce object.'),
    '#options' => $types,
    '#default_value' => array(),
    '#required' => TRUE,
    '#ajax' => array(
      'callback' => 'salesforce_mapping_form_callback',
      'wrapper' => 'salesforce_mapping_form'
    )
  );

  if (isset($form_state['values']['drupal_type'])) {
    $bundles = array('' => t('-- Select --'));
    $info  = entity_get_info($form_state['values']['drupal_type']);
    foreach ($info['bundles'] as $key => $bundle) {
      $bundles[$key] = $bundle['label'];
    }
    $form['drupal_entity']['drupal_bundle'] = array(
      '#title' => t('Drupal Entity Bundle'),
      '#type' => 'select',
      '#required' => TRUE,
      '#description' => t('Select a Drupal entity bundle to map to a Salesforce object.'),
      '#options' => $bundles,
      '#default_value' => array(),
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

  $form['salesforce_object']['salesforce_type'] = array(
    '#title' => t('Salesforce object'),
    '#type' => 'select',
    '#description' => t('Select a Salesforce object to map.'),
    '#default_value' => '',
    '#options' => $sfobject_options,
    '#ajax' => array(
      'callback' => 'salesforce_mapping_form_callback',
      'wrapper' => 'salesforce_mapping_form'
    ),
    '#required' => TRUE
  );

  $salesforce_object_type = &$form_state['values']['salesforce_type'];
  if (!empty($salesforce_object_type) || isset($mapping)) {
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
      '#options' => $sf_types,
      '#required' => TRUE,
    );

    $form['salesforce_field_mappings'] = array(
      '#title' => t('Field mappings'),
      '#type' => 'fieldset',
      '#tree' => TRUE
    );

    $drupal_fields = salesforce_mapping_drupal_fields($form_state['values']['drupal_type'], $form_state['values']['drupal_bundle']);

    $sf_fields = array('' => t('-- Select --'));
    foreach ($sfobject['fields'] as $sf_field) {
      $sf_fields[$sf_field['name']] = $sf_field['label'];
    }

    $form['salesforce_field_mappings']['field_mappings_table'] = array(
      '#theme' => 'salesforce_fieldmap_form_table',
      '#tree' => TRUE
    );

    $mappings_count = isset($form_state['salesforce_mappings_count']) ? $form_state['salesforce_mappings_count'] : 0;
    if ($form_state['triggering_element']['#name'] == 'salesforce_add_field') {
      $mappings_count++;
      $form_state['salesforce_mappings_count'] = $mappings_count;
    }

    for ($delta = 0; $delta <= $mappings_count; $delta++) {
      $form['salesforce_field_mappings']['field_mappings_table'][$delta] = array(
        'drupal_field' => array(
          '#type' => 'select',
          '#description' => t('Select a Drupal field or property to map to a Salesforce field.'),
          '#options' => $drupal_fields
        ),
        'salesforce_field' => array(
          '#type' => 'select',
          '#description' => t('Select a Salesforce field to map.'),
          '#options' => $sf_fields
        )
      );
    }

    $form['salesforce_field_mappings']['salesforce_add_field'] = array(
      '#value' => t('Add field mapping'),
      '#name' => 'salesforce_add_field',
      '#type' => 'button',
      '#description' => t('Add one or more field to configure a mapping for.'),
//      '#limit_validation_errors' => array(),
      '#ajax' => array(
        'callback' => 'salesforce_mapping_form_callback',
        'wrapper' => 'salesforce_mapping_form'
      )
    );
  }

  return $form;
}

function salesforce_mapping_drupal_fields($entity_type, $bundle) {
  $options = array();
  $fields = field_info_instances($entity_type, $bundle);
  foreach ($fields as $field_name => $field_info) {
    $options['fields'][$field_name] = $field_info['label'];
  }

  $properties = entity_get_all_property_info($entity_type);
  foreach ($properties as $key => $property) {
    $options['properties'][$key] = $property['label'];
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

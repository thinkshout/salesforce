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

  $form['drupal_entity'] = array(
    '#title' => t('Drupal entity'),
    '#type' => 'fieldset',
    '#attributes' => array(
      'id' => 'drupal_entity'
    )
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
      'wrapper' => 'drupal_entity'
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
        'wrapper' => 'salesforce_object'
      )
    );
  }

  $form['salesforce_wrapper'] = array(
    '#type' => 'item',
    '#prefix' => '<div id="salesforce-wrapper">',
    '#suffix' => '</div>'
  );

  $form['salesforce_wrapper']['salesforce_object'] = array(
    '#title' => t('Salesforce object'),
    '#type' => 'fieldset',
    '#attributes' => array(
      'id' => 'salesforce_object'
    )
  );

  $sfapi = salesforce_get_api();

  if ((isset($form_state['values']['drupal_bundle']) && !empty($form_state['values']['drupal_bundle'] )) || isset($mapping)) {
    $sobjects = array('' => t('-- Select --'));
    foreach ($sfapi->objects() as $object) {
      $sobjects[$object['name']] = $object['label'];
    }

    $form['salesforce_wrapper']['salesforce_object']['salesforce_type'] = array(
      '#title' => t('Salesforce object'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce object to map to !bundle', array(
        '!bundle' => $form_state['values']['drupal_bundle']
      )),
      '#default_value' => '',
      '#options' => $sobjects,
      '#ajax' => array(
        'callback' => 'salesforce_mapping_form_callback',
        'wrapper' => 'salesforce-wrapper'
      )
    );
  }

  if ((isset($form_state['values']['salesforce_type']) && !empty($form_state['values']['salesforce_type'] )) || isset($mapping)) {
    $object = $sfapi->objectDescribe($form_state['values']['salesforce_type']);
    $sf_types = array('' => t('-- Select --'));
    foreach ($object['recordTypeInfos'] as $type) {
      $sf_types[$type['recordTypeId']] = $type['name'];
    }
    $form['salesforce_wrapper']['salesforce_object']['salesforce_record_type'] = array(
      '#title' => t('Salesforce record type'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce record type to map.'),
      '#options' => $sf_types
    );

    $sf_fields = array('' => t('-- Select --'));
    foreach ($object['fields'] as $sf_field) {
      $sf_fields[$sf_field['name']] = $sf_field['label'];
    }
    $form['salesforce_wrapper']['salesforce_field_mappings'] = array(
      '#title' => t('Field mappings'),
      '#type' => 'fieldset',
      '#attributes' => array(
        'id' => 'salesforce_field_mappings'
      ),
      '#states' => array(
        'invisible' => array(
          ':input[name="salesforce_type"]' => array('value' => ''),
        )
      )
    );
    $form['salesforce_wrapper']['salesforce_field_mappings']['salesforce_fields'] = array(
      '#title' => t('Salesforce fields'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce field to map.'),
      '#options' => $sf_fields
    );
    $form['salesforce_wrapper']['salesforce_field_mappings']['salesforce_add_field'] = array(
      '#value' => t('Add field'),
      '#type' => 'submit',
      '#description' => t('Add one or more field to configure a mapping for.'),
    );
  }

  return $form;
}

/**
 * Ajax callback for salesforce_mapping_form().
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function salesforce_mapping_form_callback($form, &$form_state) {
  switch ($form_state['triggering_element']['#name']) {
    case 'drupal_type':
      $wrapper = 'drupal_entity';
      break;
    case 'drupal_bundle':
      $wrapper = 'salesforce_wrapper';
      break;
    case 'salesforce_type':
      $wrapper = 'salesforce_wrapper';
      break;
  }

  // return $form;
  return $form[$wrapper];
}

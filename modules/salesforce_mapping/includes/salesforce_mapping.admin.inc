<?php

function salesforce_mapping_form($form, &$form_state, $mapping = NULL) {

  $types = array('' => t('-- Select --'));
  foreach (entity_get_info() as $type => $info) {
    $types[$type] = $info['label'];
  }
  $form['drupal_type'] = array(
    '#title' => t('Drupal Entity Type'),
    '#type' => 'select',
    '#description' => t('Select a Drupal entity type to map to a Salesforce object.'),
    '#options' => $types,
    '#default_value' => array(),
    '#required' => TRUE,
    '#ajax' => array(
      'callback' => 'salesforce_mapping_form_callback',
      'wrapper' => 'drupal-bundle-wrapper'
    )
  );

  $form['bundle_wrapper'] = array(
    '#prefix' => '<div id="drupal-bundle-wrapper">',
    '#suffix' => '</div>',
  );
  if (isset($form_state['values']['drupal_type'])) {
    $bundles = array('' => t('-- Select --'));
    $info  = entity_get_info($form_state['values']['drupal_type']);
    foreach ($info['bundles'] as $key => $bundle) {
      $bundles[$key] = $bundle['label'];
    }
    $form['bundle_wrapper']['drupal_bundle'] = array(
      '#title' => t('Drupal Entity Bundle'),
      '#type' => 'select',
      '#required' => TRUE,
      '#description' => t('Select a Drupal entity bundle to map to a Salesforce object.'),
      '#options' => $bundles,
      '#default_value' => array(),
      '#ajax' => array(
        'callback' => 'salesforce_mapping_form_callback',
        'wrapper' => 'object_wrapper'
      )
    );
  }

  $sfapi = salesforce_get_api();

  $form['object_wrapper'] = array(
    '#prefix' => '<div id="object_wrapper">',
    '#suffix' => '</div>',
  );
  if ((isset($form_state['values']['drupal_bundle']) && !empty($form_state['values']['drupal_bundle'] )) || isset($mapping)) {
    $sobjects = array('' => t('-- Select --'));
    foreach ($sfapi->objects() as $object) {
      $sobjects[$object['name']] = $object['label'];
    }

    $form['object_wrapper']['salesforce_object'] = array(
      '#title' => t('Salesforce object'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce object to map to !bundle', array(
        '!bundle' => $form_state['values']['drupal_bundle']
      )),
      '#default_value' => '',
      '#options' => $sobjects,
      '#ajax' => array(
        'callback' => 'salesforce_mapping_form_callback',
        'wrapper' => 'fields_wrapper'
      )
    );
  }

  $form['fields_wrapper'] = array(
    '#prefix' => '<div id="fields_wrapper">',
    '#suffix' => '</div>',
  );
  if ((isset($form_state['values']['salesforce_object']) && !empty($form_state['values']['salesforce_object'] )) || isset($mapping)) {
    $object = $sfapi->objectDescribe($form_state['values']['salesforce_object']);
    foreach ($object['fields'] as $sf_field) {
      $sf_fields[$sf_field['name']] = $sf_field['label'];
    }
    $form['fields_wrapper']['salesforce_fields'] = array(
      '#title' => t('Salesforce fields'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce field to map.'),
      '#options' => $sf_fields
    );
  }

  return $form;
}

function salesforce_mapping_form_callback($form, &$form_state) {
  switch ($form_state['triggering_element']['#name']) {
    case 'drupal_type':
      $wrapper = 'bundle_wrapper';
      break;
    case 'drupal_bundle':
      $wrapper = 'object_wrapper';
      break;
    case 'salesforce_object':
      $wrapper = 'fields_wrapper';
      break;
  }

  return $form[$wrapper];
}

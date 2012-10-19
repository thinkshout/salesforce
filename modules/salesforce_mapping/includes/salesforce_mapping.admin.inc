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

  $sobjects = array('' => t('-- Select --'));
  foreach ($sfapi->objects() as $object) {
    $sobjects[$object['name']] = $object['label'];
  }

  $form['salesforce_object']['salesforce_type'] = array(
    '#title' => t('Salesforce object'),
    '#type' => 'select',
    '#description' => t('Select a Salesforce object to map.'),
    '#default_value' => '',
    '#options' => $sobjects,
    '#ajax' => array(
      'callback' => 'salesforce_mapping_form_callback',
      'wrapper' => 'salesforce_mapping_form'
    ),
    '#required' => TRUE
  );

  if ((isset($form_state['values']['salesforce_type']) && !empty($form_state['values']['salesforce_type'] )) || isset($mapping)) {
    $object = $sfapi->objectDescribe($form_state['values']['salesforce_type']);
    $sf_types = array('' => t('-- Select --'));
    foreach ($object['recordTypeInfos'] as $type) {
      $sf_types[$type['recordTypeId']] = $type['name'];
    }
    $form['salesforce_object']['salesforce_record_type'] = array(
      '#title' => t('Salesforce record type'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce record type to map.'),
      '#options' => $sf_types,
      '#required' => TRUE,
    );

    $sf_fields = array('' => t('-- Select --'));
    foreach ($object['fields'] as $sf_field) {
      $sf_fields[$sf_field['name']] = $sf_field['label'];
    }
    $form['salesforce_field_mappings'] = array(
      '#title' => t('Field mappings'),
      '#type' => 'fieldset',
      '#attributes' => array(
        'id' => 'salesforce_field_mappings'
      ),
    );
    $form['salesforce_field_mappings']['salesforce_field'] = array(
      '#title' => t('Salesforce fields'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce field to map.'),
      '#options' => $sf_fields
    );

    $drupal_fields = salesforce_mapping_drupal_fields($form_state['values']['drupal_type'], $form_state['values']['drupal_bundle']);
    $form['salesforce_field_mappings']['drupal_field'] = array(
      '#title' => t('Drupal fields & properties'),
      '#type' => 'select',
      '#description' => t('Select a Drupal field or property to map to a Salesforce field.'),
      '#options' => $drupal_fields
    );
    $form['salesforce_field_mappings']['salesforce_add_field'] = array(
      '#value' => t('Add field mapping'),
      '#type' => 'button',
      '#description' => t('Add one or more field to configure a mapping for.'),
      '#ajax' => array(
        'callback' => 'salesforce_mapping_form_callback',
        'wrapper' => 'salesforce_mapping_form'
      )
    );

    $table = array(
      '#theme' => 'theme_salesforce_fieldmap_form_table',
      '#tree' => TRUE,
      '#header' => array(
        t('Salesforce field'),
        t('Drupal field'),
        t('Settings'),
        t('Remove')
      ),
      '#parent_options' => array(),
      '#rows' => array()
    );

//    if (!empty($form_state['values']['drupal_field'])) {
//      $table['#rows'][] = array(
//        'drupal_field' => array(
//          '#markup' => $form_state['values']['drupal_field'],
//        ),
//        'salesforce_field' => array(
//          '#markup' => $form_state['values']['drupal_field'],
//        ),
//        'settings' => array(
//        ),
//        'delete' => array(
//          '#type' => 'link',
//          '#title' => t('delete'),
//          '#options' => array('attributes' => array('title' => t('Delete instance.'))),
//        ),
//      );
//    }

    $form['field_mappings_table'] = $table;

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
    case 'op':
      $form['field_mappings_table']['#rows'][] = array(
        'drupal_field' => $form_state['values']['drupal_field'],
        'salesforce_field' => $form_state['values']['salesforce_field'],
        'settings' => array(),
        'delete' => array(),
      );

      break;
  }

   return $form;
}


/**
 * Themes the field associations on a fieldmap edit form into a table.
 */
function theme_salesforce_fieldmap_form_table($variables) {
  $form = $variables['form'];
  // Build the header array.
  $header = array();

  foreach (element_children($form['header']) as $element) {
    $header[] = drupal_render($form['header'][$element]);
  }
  // Add the operations column. (Currently "Remove" is the only operation.
  $header[] = 'Operations';

  // Build the rows array.
  $rows = array();
  foreach (element_children($form['rows']) as $element) {
    $rows[] = array(
      array(
        'data' => drupal_render($form['rows'][$element]['target']),
        'class' => 'target-cell',
      ),
      array(
        'data' => drupal_render($form['rows'][$element]['source']),
        'class' => 'source-cell',
      ),
      array(
       'data' => drupal_render($form['rows'][$element]['remove']),
      )
    );
  }

  // Add a message if no rows were found.
  if (empty($rows)) {
    $rows[] = array(
      array(
        'data' => t('No fields set.'),
        'colspan' => 3,
      ),
    );
  }

  // Build the attributes array.
  $attributes = array();

  // Build the caption.
  $caption = NULL;

  if (isset($form['caption'] )) {
    $caption = drupal_render_children($form['caption']);
  }
  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => $attributes, 'caption' => $caption));
}

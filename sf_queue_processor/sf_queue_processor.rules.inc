<?php

/**
 * @file
 * Functions for rules integration.
 */

/**
 * Implements hook_rules_event_info().
 */
function sf_queue_processor_rules_event_info() {

  $arguments = array(
    'type' => array(
      'type' => 'value',
      'label' => t('The type of object that failed to export'),
    ),
    'oid' => array(
      'type' => 'value',
      'label' => t('The id of the object that failed to export'),
    ),
    'action' => array(
      'type' => 'value',
      'label' => t('The action that was being taken when the object failed (create/upsert/update/delete'),
    ),
    'item' => array(
      'type' => 'value',
      'label' => t('The batch item that was being exported to Salesforce'),
    ),
    'error' => array(
      'type' => 'value',
      'label' => t('The error that caused the object to fail'),
    ),
  );

  return array(
    'sf_queue_processor_export_error' => array(
      'label' => t('An object fails to export to Salesforce'),
      'module' => 'Salesforce Queue',
      'arguments' => $arguments,
    ),
  );
}

/**
 * Implements hook_condition_info().
 */
function sf_queue_processor_rules_condition_info() {
  return array(
    'sf_queue_processor_condition_drupal_type' => array(
      'label' => t('Check the type of item that failed'),
      'arguments' => array(
        'type' => array(
          'type' => 'value',
          'label' => t('The type of item that failed'),
        ),
      ),
      'module' => 'Salesforce',
    ),
    'sf_queue_processor_condition_status_code' => array(
      'label' => t('Check the status code an error'),
      'arguments' => array(
        'error' => array(
          'type' => 'value',
          'label' => t('The status code of the returned error'),
        ),
        'status_code' => array(
          'type' => 'string',
          'label' => t('The Salesforce error status code'),
        ),
      ),
      'module' => 'Salesforce',
    ),
    'sf_queue_processor_condition_object_field' => array(
      'label' => t('Check the name of a field'),
      'arguments' => array(
        'error' => array(
          'type' => 'value',
          'label' => t('The error that was returned from Salesforce'),
        ),
        'field' => array(
          'type' => 'string',
          'label' => t('Salesforce field name'),
        ),
      ),
      'module' => 'Salesforce',
    ),
  );
}

/**
 * Implements hook_rules_action_info().
 */
function sf_queue_processor_rules_action_info() {
  return array(
    'sf_queue_processor_add_to_queue_action' => array(
      'label' => t('Add item to Salesforce export queue'),
      'module' => 'Salesforce',
      'arguments' => array(
        'type' => array(
          'type' => 'value',
          'label' => t('The type of Drupal object to add to the queue'),
        ),
        'oid' => array(
          'type' => 'value',
          'label' => t('The id of the Drupal object to add to the queue'),
        ),
        'action' => array(
          'type' => 'value',
          'label' => t('The Salesforce operation to perform on the object'),
        ),
      ),
    ),
  );
}

/**
 * Settings form for sf_queue_processor_condition_drupal_type action.
 */
function sf_queue_processor_condition_drupal_type_form($settings, &$form) {
  $settings += array('drupal_type' => '');
  $processing_order = variable_get('queue_processing_order', array('user', 'donation', 'webform', 'node'));
  $options = array();
  foreach ($processing_order as $type) {
    $options[$type] = $type;
  }

  $form['settings']['drupal_type'] = array(
    '#type' => 'select',
    '#title' => t('Drupal type'),
    '#options' => $options,
    '#default_value' => $settings['drupal_type'],
    '#description' => t('Select the Drupal object type.'),
  );
}

/**
 * Condition to check the Drupal object type.
 *
 * @param $type
 *   The type of object to check.
 * @param $settings
 *   The configured rule settings.
 */
function sf_queue_processor_condition_drupal_type($type, $settings) {
  return $type == $settings['drupal_type'];
}

/**
 * Rules action to add an item to the export queue.
 *
 * @param $type
 *   The type of Drupal object to add (user, donation, etc).
 * @param $oid
 *   The id of the Drupal object to add.
 * @param $action
 *   The Salesforce operation to perform on the object (create/upsert/update/delete).
 */
function sf_queue_processor_add_to_queue_action($type, $oid, $action) {
  // if action is update, lets make sure the item is sync'd up correctly
  $map = salesforce_api_id_load($type, $oid);
  if (empty($map)) {
    $action = 'upsert';
  }
  sf_queue_insert($oid, $type, $action);
}

/**
 * Rules condition function to compare Salesforce error status codes.
 */
function sf_queue_processor_condition_status_code($error, $status_code) {
  return $error->statusCode == $status_code;
}

/**
 * Rules condition function to compare Salesforce object field names.
 */
function sf_queue_processor_condition_object_field($error, $field) {
  if (stristr($error->fields, $field)) {
    return TRUE;
  }

  return FALSE;
}

/**
 * Helper function for event variables.
 *
 * @return
 *   All available variables for the rules events provided by the queue processor
 */
function _queue_processor_error_event_variables() {
  return array(
    'status_code' => array(
      'type' => 'string',
      'label' => t('The Salesforce error status code returned from the API'),
    ),
    'field' => array(
      'type' => 'string',
      'label' => t('The field in the Salesforce object that caused the error'),
    ),
  );
}


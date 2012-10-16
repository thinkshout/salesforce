<?php

/**
 * @file
 * Drush integration for Salesforce.
 */

/**
 * Implementation of hook_drush_command().
 */
function salesforce_drush_command() {
  $items['sf-rest-version'] = array(
    'description' => 'Displays information about the current REST API version',
    'aliases' => array('sfrv'),
  );
  $items['sf-list-objects'] = array(
    'description' => 'List the objects that are available in your organization and available to the logged-in user.',
    'aliases' => array('sflo'),
  );
  $items['sf-describe-object'] = array(
    'description' => 'Retrieve all the metadata for an object, including information about each field, URLs, and child relationships.',
    'aliases' => array('sfdo'),
    'arguments' => array(
      'object' => 'The object name in Salesforce.',
    ),
    'options' => array(
      'fields' => 'Display information about fields that are part of the object.',
    ),
  );
  $items['sf-list-resources'] = array(
    'description' => 'List the resources available for the specified API version. It provides the name and URI of each resource.',
    'aliases' => array('sflr'),
  );
  return $items;
}

/**
 * List the resources available for the specified API version. It provides the
 * name and URI of each resource.
 */
function drush_salesforce_sf_list_resources() {
  $salesforce = _drush_salesforce_drush_get_api();
}

function drush_salesforce_sf_describe_object($object = NULL) {
  if (!$object) {
    return drush_log('Please specify an object as an argument.', 'error');
  }
  $salesforce = _drush_salesforce_drush_get_api();

  $object = $salesforce->objectDescribe($object);

  // Return if we cannot load any data
  if (!is_array($object)) {
    return drush_log(dt('Could not load data for object !object', array('!object' => $object)), 'error');
  }
  // Display only information about fields for an option
  if (drush_get_option('fields')) {
    drush_print_r($object['fields']);
    drush_print_r($object['childReleationships']);
    return;
  }

  // Display information about the object
  $rows[] = array('Name', 'Fields', 'childRelationships', 'searchable', 'createable', 'deletable', 'mergeable', 'queryable');
  $rows[] = array(
    $object['name'],
    isset($object['fields']) ? count($object['fields']) : 0,
    isset($object['childRelationships']) ? count($object['childRelationships']) : 0,
    ($object['searchable'] == 1) ? 'TRUE' : 'FALSE',
    ($object['createable'] == 1) ? 'TRUE' : 'FALSE',
    ($object['deletable'] == 1) ? 'TRUE' : 'FALSE',
    ($object['mergeable'] == 1) ? 'TRUE' : 'FALSE',
    ($object['queryable'] == 1) ? 'TRUE' : 'FALSE',
  );
  drush_print_table($rows, TRUE);
}

/**
 * Displays information about the REST API version the site is using.
 */
function drush_salesforce_sf_rest_version() {
  $salesforce = _drush_salesforce_drush_get_api();
  if (isset($salesforce->rest_api_version)) {
    $rows[] = array('Salesforce', 'Value');
    foreach ($salesforce->rest_api_version as $key => $value) {
      $rows[] = array($key, $value);
    }
    $rows[] = array('login url', $salesforce->login_url);
    drush_print_table($rows, TRUE);
  } else {
    drush_log('Could not obtain information about the current REST API version!', 'error');
  }
}

function _drush_salesforce_drush_get_api() {
  if ($salesforce = salesforce_get_api()) {
    return $salesforce;
  }
}

/**
 * List the objects that are available in your organization and available to
 * the logged-in user.
 */
function drush_salesforce_sf_list_objects() {
  $salesforce = _drush_salesforce_drush_get_api();
  if ($objects = $salesforce->objects()) {
    drush_print('The following objects are available in your organization and available to the logged-in user.');
    $rows[] = array('Name', 'Label', 'Label Plural');
    foreach ($objects as $object) {
      $rows[] = array($object['name'], $object['label'], $object['labelPlural']);
    }
    drush_print_table($rows, TRUE);
  } else {
    drush_log('Could not load any information about available objects.', 'error');
  }

}

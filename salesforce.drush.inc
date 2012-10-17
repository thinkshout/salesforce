<?php

/**
 * @file
 * Drush integration for Salesforce.
 */

/**
 * Implements hook_drush_command().
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
      'field-data' => 'Display information about a specific field that is part of an object',
    ),
  );
  $items['sf-list-resources'] = array(
    'description' => 'List the resources available for the specified API version. It provides the name and URI of each resource.',
    'aliases' => array('sflr'),
  );
  $items['sf-session-id'] = array(
    'description' => 'Obtain a Session ID for use with the Salesforce REST API.',
    'options' => array(
      'username' => 'The Salesforce username',
      'password' => 'The password and token associated with the Salesforce username',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
  );
  return $items;
}

/**
 * List the resources available for the specified API version.
 *
 * This command provides the name and URI of each resource.
 *
 * @TODO implement function
 */
function drush_salesforce_sf_list_resources() {
  $salesforce = _drush_salesforce_drush_get_api();
}

/**
 * Describes a Salesforce object.
 *
 * Use the --fields option to display information about the fields of an object,
 * or the --field-data option to display information about a single field in an
 * object.
 *
 * @param string $object_name
 *   The name of a Salesforce object to query.
 */
function drush_salesforce_sf_describe_object($object_name = NULL) {
  if (!$object_name) {
    return drush_log('Please specify an object as an argument.', 'error');
  }
  $salesforce = _drush_salesforce_drush_get_api();

  $object = $salesforce->objectDescribe($object_name);

  // Return if we cannot load any data.
  if (!is_array($object)) {
    return drush_log(dt('Could not load data for object !object', array('!object' => $object)), 'error');
  }
  // Display only information about fields for an option,
  if (drush_get_option('fields')) {
    $rows = array(array('Name', 'Type', 'Label'));
    foreach ($object['fields'] as $field) {
      $rows[] = array($field['name'], $field['type'], $field['label']);
    }
    drush_print_r($rows);
    drush_print_table($rows, TRUE);
    return;
  }

  // Display only information about a specific field.
  if ($fieldname = drush_get_option('field-data')) {
    $field_data = NULL;
    foreach ($object['fields'] as $field) {
      if ($field['name'] === $fieldname) {
        $field_data = $field;
        break;
      }
    }
    if (!$field_data) {
      drush_log(dt('Could not load data for field !field on !object object', array('!field' => $fieldname, '!object' => $object_name)), 'error');
    }
    else {
      drush_print_r($field);
    }

    return;
  }

  // Display information about the object.
  // @TODO add remaining field objects?
  $rows = array();
  $rows[] = array('Name', $object['name']);
  $rows[] = array('Fields', isset($object['fields']) ? count($object['fields']) : 0);
  $rows[] = array('Child Relationships', isset($object['childRelationships']) ? count($object['childRelationships']) : 0);
  $rows[] = array('Searchable', ($object['searchable'] == 1) ? 'TRUE' : 'FALSE');
  $rows[] = array('Creatable', ($object['createable'] == 1) ? 'TRUE' : 'FALSE');
  $rows[] = array('Deletable', ($object['deletable'] == 1) ? 'TRUE' : 'FALSE');
  $rows[] = array('Mergeable', ($object['mergeable'] == 1) ? 'TRUE' : 'FALSE');
  $rows[] = array('Queryable', ($object['queryable'] == 1) ? 'TRUE' : 'FALSE');
  drush_print_table($rows);
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
  }
  else {
    drush_log('Could not obtain information about the current REST API version!', 'error');
  }
}

/**
 * Wrapper around salesforce_get_api().
 *
 * If salesforce_get_api() does not return a connection to Salesforce,
 * this function can prompt the user for username/password to obtain a new
 * token.
 *
 * @TODO implement this function
 */
function _drush_salesforce_drush_get_api() {
  if ($salesforce = salesforce_get_api()) {
    return $salesforce;
  }
}

/**
 * List Salesforce objects.
 *
 * This command lists Salesforce objects that are available in your organization
 * and available to the logged-in user.
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
  }
  else {
    drush_log('Could not load any information about available objects.', 'error');
  }

}

/**
 * Obtain a new session ID using Salesforce username and password.
 */
function drush_salesforce_sf_session_id() {
  // Check to see if we already have a session ID.
  if ($_SESSION['salesforce_access_token']) {
    if (!drush_confirm('You already have a session ID for interacting with Salesforce. Are you sure you want to obtain a new one?')) {
      return;
    }
  }
  $username = drush_get_option('username');
  $password = drush_get_option('password');
  if (!$username && !$password) {
    drush_print('To obtain a new session ID, you must enter your username, and your password with your token.');
  }

  if (!$username) {
    $username = drush_prompt('Login');
  }
  if (!$password) {
    $password = drush_prompt('Password and Token');
  }
  $salesforce = salesforce_get_api();
  $login_xml = _drush_salesforce_soap_login_xml($username, $password);
  $login_url = 'https://login.salesforce.com/services/Soap/u/26.0';
  // $ret = $salesforce->httpRequest($url, $login_xml, $headers);
  $headers = array(
    'SOAPAction' => 'login',
    'Content-Type' => 'text/xml',
  );
  $raw_xml = drupal_http_request($login_url, array(
    'headers' => $headers,
    'method' => 'POST',
    'data' => $login_xml,
    )
  );
  if (isset($raw_xml->error)) {
    return drush_log(dt('Failed to obtain a session ID. Error returned: code !code !error', array('!code' => $raw_xml->code, '!error' => $raw_xml->error)), 'error');
  }
  $dom = new DOMDocument();
  $dom->loadXML($raw_xml->data);
  $session_id = $dom->getElementsByTagName('sessionId')->item(0)->textContent;
  if ($session_id) {
    $_SESSION['salesforce_access_token'] = $session_id;
    drush_log('Obtained and set new session ID.', 'success');
  }
  else {
    return drush_log('Failed to obtain a session ID.', 'error');
  }
}

/**
 * Returns XML for Salesforce SOAP login method.
 */
function _drush_salesforce_soap_login_xml($username, $password) {
  return '<?xml version="1.0" encoding="utf-8" ?>
  <env:Envelope xmlns:xsd="http://www.w3.org/2001/XMLSchema"
      xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xmlns:env="http://schemas.xmlsoap.org/soap/envelope/">
    <env:Body>
      <n1:login xmlns:n1="urn:partner.soap.sforce.com">
        <n1:username>' . $username . '</n1:username>
        <n1:password>' . $password . '</n1:password>
      </n1:login>
    </env:Body>
  </env:Envelope>';
}

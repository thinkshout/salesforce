<?php
// $Id$

/**
 * @file
 * Drush integration for Salesforce. Provides commands to examine and query
 * Salesforce data.
 */

/**
 * Implementation of hook_drush_help().
 */
function salesforce_api_drush_help($section) {
  switch ($section) {
    case 'drush:soql-show-tables':
      return dt("List tables in salesforce.");
    case 'drush:soql-describe':
      $output =  dt("Examine table metadata in salesforce.") ."\n\n";
      $output .= dt("Arguments:\n  Salesforce Object    The path of the salesforce object to examine.") ."\n\n";
      $output .= dt("Examples:\n  drush soql describe Contact.fields    Display the fields on the Contact table.");
      return $output;
    case 'drush:soql':
      $output = dt("Execute a SOQL query. Note that you may need to escape quotes in your query: SELECT Email FROM Contact WHERE Email != \\'\\'") ."\n\n";
      $output .= dt("Arguments:\n  SOQL Query    A salesforce soql query to execute against salesforce.") ."\n\n";
      $output .= dt("Examples:\n  drush soql select Id, Name from Contact    Query the Contact table.");
      return $output;
  }
}

/**
 * Implementation of hook_drush_command().
 */
function salesforce_api_drush_command() {
  $items['soql-show-tables'] = array(
    'description' => 'List tables in salesforce.',
    'aliases' => array('sf-show'),
  );
  $items['soql-describe'] = array(
    'description' => 'Describe an individual table in salesforce.',
    'aliases' => array('sf-desc'),
  );
  $items['soql'] = array(
    'description' => 'Execute a SOQL query.',
    'aliases' => array('sf-q'),
  );
  return $items;
}

/**
 * Duplicates the functionality of mysql's SHOW TABLES
 */
function drush_salesforce_api_soql_show_tables() {
  _salesforce_api_drush_check_soap();
  if ($sf = salesforce_api_connect()) {
    try {
      $response = salesforce_api_describeGlobal();
    }
    catch (Exception $e) {
      drush_print($e->faultstring);
      return;
    }
    $rows = array(array(dt('Label'), dt('Name')));
    $rows[] = array('------------', '------------');
    foreach ($response->types as $type) {
      $rows[] = array($type->label, $type->name);
    }
    drush_print_table($rows, TRUE);
    drush_print("\n" . dt('!count rows in set.', array('!count' => (count($rows)-1))));
  }
  else {
    drush_print('Could not connect to salesforce.');
  }
}

/**
 * Examine a salesforce table. Allows for deep inspection using dot syntax
 * similar to sql databases.
 *
 * For example to see fields on contact: 'contact.fields'
 * To examine the first field on a contact 'contact.fields.1'
 */
function drush_salesforce_api_soql_describe() {
  _salesforce_api_drush_check_soap();
  $tables = func_get_args();
  if ($sf = salesforce_api_connect()) {
    foreach ($tables as $t) {
      $info = explode('.', $t);
      try {
        $table = array_shift($info);
        $response = salesforce_api_describeSObject($table);
        if (!empty($info)) {
          $i = array_shift($info);
          while (isset($i)) {
            if (is_array($response)) {
              $response = $response[$i];
            }
            else {
              $response = $response->$i;
            }
            $_i = $i; // Save the last $i value for later.
            $i = array_shift($info);
          }
        }
        // If fields were requested give them some special handling.
        if ($_i == 'fields') {
          $rows = array(array('Key', 'Type', 'Name', 'Label'));
          _salesforce_api_drush_get_fields($response, $rows);
          drush_print_table($rows, TRUE);
        }
        else {
          $rows = array(array(dt('Key'), dt('Value')));
          _salesforce_api_drush_get_info($response, $rows);
          drush_print_table($rows, TRUE);
        }
      }
      catch (Exception $e) {
        drush_print($e->faultstring);
      }
    }
  }
  else {
    drush_print('Could not connect to salesforce.');
  }
}

/**
 * Walk a response element and retrieve useful info.
 */
function _salesforce_api_drush_get_info($response, &$rows) {
  foreach ($response as $k => $v) {
    if (!is_array($v) && !is_object($v)) {
      $rows[] = array($k, $v);
    }
    else {
      if (is_object($v) && isset($v->name)) {
        // For field like objects.
        $label = $v->name;
      }
      elseif (is_object($v) && isset($v->childSObject)) {
        // For relationships
        $label = $v->childSObject . '.' . $v->field;
      }
      elseif (is_object($v) && isset($v->label)) {
        // For picklist values
        $label = $v->label;
      }
      else {
        $label = '';
      }
      $rows[] = array($k ." [+]", $label);
    }
  }
}

/**
 * Recurse into a salesforce API result and retrieve fields into a table row.
 */
function _salesforce_api_drush_get_fields($response, &$rows) {
  foreach ($response as $key => $item) {
    if (is_object($item)) {
      $item = (array) $item;
    }
    if (is_array($item)) {
      if (!empty($item['type'])) {
        $row = array();
        $row[] = $key ." [+]";
        $row[] = $item['type'];
        $row[] = $item['name'];
        $row[] = $item['label'];

        $rows[] = $row;
      }
      else {
        _salesforce_api_drush_get_fields($item, $rows);
      }
    }
  }
}

/**
 * Execute a SOQL query.
 */
function drush_salesforce_api_soql() {
  _salesforce_api_drush_check_soap();
  $args = func_get_args();
  $command = implode(' ', $args);
  if ($sf = salesforce_api_connect()) {
    try {
      timer_start('soql_query');
      $result = $sf->client->query($command);
      $timer = timer_stop('soql_query');
      foreach ($result->records as $r) {
        if (!isset($rows)) {
          $header = array();
          foreach ($r as $k => $v) {
            $header[] = $k;
          }
          $rows = array($header);
        }
        $row = array();
        foreach ($r as $k => $v) {
          $row[] = $v;
        }
        $rows[] = $row;
      }
      drush_print_table($rows, TRUE);
      drush_print("\n" . dt('!count rows in set (!time sec)',
        array('!count' => $result->size, '!time' => number_format($timer['time']/1000, 2))));
    }
    catch (Exception $e) {
      drush_print($e->faultstring);
    }
  }
  else {
    drush_print('Could not connect to salesforce.');
  }
}

/**
 * Safety check! Make sure the we have access to SOAP, if not try to give the
 * user a helpful error message.
 */
function _salesforce_api_drush_check_soap() {
  if (!extension_loaded('soap')) {
    drush_die("Have you enabled SOAP? If you can connect through the browser, make sure your command line PHP binary is the same as your web server's PHP binary.");
  }
}

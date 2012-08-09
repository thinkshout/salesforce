<?php

/**
 * @file
 * Drush integration for Salesforce Import module. Provides commands to import
 * Salesforce data.
 */

/**
 * Implementation of hook_drush_help().
 */
function sf_import_drush_help($section) {
  switch ($section) {
    case 'drush:sf-fieldmap':
      $output = dt("List available salesforce fieldmaps or shows information on a given fieldmap.\n\n");
      $output .= dt("Optional argument: Key of salesforce fieldmap");
      return $output;
    case 'drush:sf-import':
      $output = dt("Initiate a salesforce import.") . "\n\n";
      $output .= dt("Argument: Key of salesforce fieldmap");
      return $output;
    case 'drush:sf-get-updated':
      $output = dt("Get updated Salesforce records.") . "\n\n";
  }
}

/**
 * Implementation of hook_drush_command().
 */
function sf_import_drush_command() {
  $items['sf-fieldmap'] = array(
    'description' => 'List available salesforce fieldmaps or shows information on a given fieldmap',
    'arguments' => array(
      'fieldmap' => 'Optional, key of fieldmap for which to retrieve information',
    ),
    'options' => array(
      '--no-sf' => 'Don\'t check the number of available records on Salesforce',
      '--where' => 'Enter any additional SOQL "Where" conditions for checking number of records on Salesforce',
    ),
  );

  $items['sf-import'] = array(
    'description' => 'Initiate a salesforce import.',
    'arguments' => array(
      'fieldmap' => 'Name of the fieldmap to import',
    ),
    'options' => array(
      '--where'  => 'Enter any additional SOQL "Where" conditions',
      '--start'  => 'Start importing at given row',
      '--count'  => 'Only import the given number of rows',
      '--no-new' => 'Don\'t import new rows (if we have linking information)',
      '--no-update' => 'Don\'t update existing rows (if we having linking information)',
      '--check-date' => 'Only import rows that have a Salesforce last modified date more
      recent than the Drupal changed date (only applies to nodes)',
    ),
  );

  $items['sf-get-updated'] = array(
    'description' => 'Get updated Salesforce records',
    'options' => array(
      '--no-process' => 'Save the updated SFIDs to sf_import_queue table, but do not process',
    )
  );

  return $items;
}

/**
 * Calls SF Import Get Updated
 */
function drush_sf_import_sf_get_updated() {

  drush_print('Checking for updated records...');
  $records = sf_import_import_records();
  if (is_array($records) && count($records) > 0) {
    drush_print("Returned " . count($records) . " record(s)");
    $table = array(array('SFID', 'Fieldmap', 'Time Imported'));
    $results = array_merge($table, $records);
    drush_print_table($results);
    $no_process = drush_get_option('no-process', FALSE);
    if ($no_process) {
      return;
    }
    drush_print("Processing records...");
    $process = _sf_import_process_records();
    if (is_array($process) && count($process) > 0) {
      drush_print("Processed " . count($process) . " record(s)");
      $table = array(array('SFID', 'OID', 'Fieldmap'));
      $results = array_merge($table, $process);
      drush_print_table($results);
    }
    else {
      drush_print('Did not process any records.');
    }
  }
  else {
    drush_print('No new records.');
  }
}

/**
 * Display available Salesforce field mappings
 */
function _drush_sf_import_show_fieldmaps() {
  $fieldmaps = salesforce_api_salesforce_field_map_load_all();

  $table = array(array('Key', 'Import', 'Description'));
  foreach ($fieldmaps as $k => $map) {
    $name = salesforce_api_fieldmap_object_label('salesforce', $map->salesforce)
            . ' => '
            . salesforce_api_fieldmap_object_label('drupal', $map->drupal);

    $table[] = array($k, $name, $map->description);
  }
  drush_print_table($table, TRUE);
}

/**
 * Info about a specific fieldmap import
 */
function drush_sf_import_sf_fieldmap($fieldmap_key = NULL) {
  if (!$fieldmap_key) {
    return _drush_sf_import_show_fieldmaps();
  }

  $map = salesforce_api_fieldmap_load($fieldmap_key);
  if (empty($map)) {
    drush_die('Invalid field map');
    return;
  }

  $out = array();

  // Display field mapping
  $out[] = array('Name', $map->name);
  $out[] = array('Description', $map->description);
  $out[] = array('Drupal type', $map->drupal);
  $out[] = array('Salesforce type', $map->salesforce);

  // Read the number of existing linked entries
  $existing = db_result(db_query("
    SELECT COUNT(oid)
    FROM   {salesforce_object_map}
    WHERE  name = '%s'",
    $map->name));


  $out[] = array('Existing linked Drupal objects', $existing);

  // Read the number of entries on SF
  if (!drush_get_option('no-sf', FALSE)) {
    _sf_import_drush_check_soap();
    try {
      $sf = salesforce_api_connect();
      $sql = "
        SELECT COUNT(Id)
        FROM " . $map->salesforce . "
      ";
      $where = drush_get_option('where', FALSE);
      if ($where) {
        $sql .= "WHERE " . $where;
      }

      $query = $sf->client->query($sql);
    } catch (Exception $e) {
      drush_set_error("Error while querying number of Salesforce entries : "
        . $e->getMessage());
      return;
    }
    $out[] = array('Rows in Salesforce', $query->records[0]->any);
  }

  drush_print('General information:');
  drush_print_table($out, FALSE);

  drush_print("\nMap information:");
  $out = array(array('Salesforce', '', 'Drupal'));
  /*
  $dr_def = salesforce_api_fieldmap_objects_load('drupal', $map->drupal);
  $sf_def = salesforce_api_fieldmap_objects_load('salesforce', $map->salesforce);
   */
  foreach ($map->fields as $sf_fieldname => $dr_fieldname) {
    $out[] = array(
      $sf_fieldname,
      '<=>',
      $dr_fieldname,
    );
  }
  drush_print_table($out, TRUE);
}

/**
 * Import data from Salesforce
 */
function drush_sf_import($fieldmap_key) {
  // Initial checks
  if (!$fieldmap_key) {
    drush_die('Please provide a mapping to use');
    return;
  }

  _sf_import_drush_check_soap();
  $map = salesforce_api_fieldmap_load($fieldmap_key);
  if (empty($map)) {
    drush_die('Invalid fieldmap');
    return;
  }

  // Read options
  $where  = drush_get_option('where', FALSE);
  $start  = intval(drush_get_option('start', 0));
  $count  = intval(drush_get_option('count', -1));
  $no_new = drush_get_option('no-new', FALSE);
  $no_update = drush_get_option('no-update', FALSE);
  $check_date = drush_get_option('check-date', FALSE);

  // Read existing entries
  $result = db_query("
    SELECT oid, sfid
    FROM   {salesforce_object_map}
    WHERE  name = '%s'",
    $map->name);

  while ($row = db_fetch_array($result)) {
    $existing[$row['sfid']] = $row['oid'];
  }

  // Read entries from salesforce
  $fields = array_keys($map->fields);
  if ($check_date && !in_array('LastModifiedDate', $fields)) {
    $fields[] = 'LastModifiedDate';
  }
  if (!in_array('Id', $fields)) {
    $fields[] = 'Id';
  }
  $soql = "
    SELECT " . implode(', ', $fields) . "
    FROM   " . $map->salesforce . "
  ";
  if ($where) {
    $soql .= " WHERE " . $where;
  }
  try {
    $sf = salesforce_api_connect();
    $query = $sf->client->query($soql);
  } catch (Exception $e) {
    drush_die("Error while querying Salesforce : " . $e->getMessage());
  }

  // Prepare for import
  $type = $map->drupal;
  if (strpos($type, 'node_') === 0) {
    $type = 'node';
  }
  $function = 'sf_' . $type . '_import';
  if (!function_exists($function)) {
    drush_die("Could not find import function for type " . $type);
  }

  $drupal_object = salesforce_api_fieldmap_objects_load('drupal', $map->drupal);
  if ($count == -1) {
    $count = count($query->records) - $start;
  }
  if ($start < 0
      || ($start + $count) > count($query->records)) {
    drush_die("Start/count exceed possible rows. Number of rows in Salesfoce : " . count($query->records));
  }

  // Run the import
  $created = 0;
  $updated = 0;
  $skipped = 0;
  $failed  = 0;
  $imported  = 0;

  for ($i = $start; $i < $start + $count; $i++) {
    $record = $query->records[$i];

    // Check if we know this record
    $exists = isset($existing[$record->Id]);
    if ($exists && $no_update) {
      drush_print("Record Id $record->Id already exists, skipping");
      $skipped++;
      continue;
    }
    if (!$exists && $no_new) {
      drush_print("Record Id $record->Id does not exist, skipping");
      $skipped++;
      continue;
    }
    if ($type == 'node' && $exists && $check_date) {
      $changed = db_result(db_query("
        SELECT changed
        FROM   {node}
        WHERE  nid = %d",
        $existing[$record->Id]
      ));
      if ($changed >= strtotime($record->LastModifiedDate)) {
        drush_print("Record Id $record->Id has not been modified (date check)");
        $skipped++;
        continue;
      }
    }

    if ($exists) {
      $existing_oid = $existing[$record->Id];
    } else {
      $existing_oid = NULL;
    }

    $oid = FALSE;
    try {
      $oid = $function($record->Id, $map->name, $existing_oid);
    } catch (Exception $e) {
      drush_log("Error importing record $record->Id : " . $e->getMessage(), 'error');
    }

    if (!$oid) {
      drush_log("Error importing record $record->Id", 'error');
      $failed++;
    }
    else {
      if (!$exists) {
        drush_print("Created object $oid");
        $created++;
      }
      else {
        drush_print("Updated object $oid");
        $updated++;
      }

      $imported++;
    }
  }

  // Report
  drush_print("Number of created objects : " . $created);
  drush_print("Number of updated objects : " . $updated);
  drush_print("Number of skipped objects : " . $skipped);
  drush_print("Number of failed imports : " . $failed);
}

/**
 * Safety check! Make sure the we have access to SOAP, if not try to give the
 * user a helpful error message.
 */
function _sf_import_drush_check_soap() {
  if (!extension_loaded('soap')) {
    drush_die("Have you enabled SOAP? If you can connect through the browser, make sure your command line PHP binary is the same as your web server's PHP binary.");
  }
}

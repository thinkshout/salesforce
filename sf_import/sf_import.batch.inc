<?php

/**
 * @file
 * Batch API related code.
 */

/**
 * Creates a batch job for a fieldmap.
 *
 * @param string $fieldmap
 *   The name of the fieldmap
 * @param array $extra
 *   An array of extra options for a given batchjob.
 */
function sf_import_create_batchjob($fieldmap, $extra = NULL) {
  $params = array('fieldmap_key' => $fieldmap, 'extra' => $extra);
  return array(
    'title' => t('Importing'),
    'operations' => array(
      array('sf_import_batchjob', $params)),
    'finished' => 'sf_import_batchjob_finalize',
    'file' => drupal_get_path('module', 'sf_import') . '/sf_import.batch.inc',
  );
}

/**
 * Finished callback for batchjob.
 *
 * @param boolean $success
 * @param array $results
 * @param $operations
 */
function sf_import_batchjob_finalize($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('Import complete.'));
    if (count($results) > 0) {
      drupal_set_message(theme('item_list', $results));
    }
  }
  else {
    drupal_set_message(t('Import failed.'));
  }
}

/**
 * Callback for sf_import_create_batchjob.
 *
 * @param string $fieldmap_key
 *   Name of the fieldmap for a batchjob.
 * @param array $extra
 * @param array $context
 */
function sf_import_batchjob($fieldmap_key, $extra, &$context) {
  // Always log in to salesforce.
  if (empty($context['sandbox'])) {
    // Do this on the first run.
    $context['sandbox'] = array();
    $context['sandbox']['progress'] = 0;
    $map = salesforce_api_fieldmap_load( $fieldmap_key );
    if (empty($map)) {
      $context['finished'] = 1;
      $context['message'] = t('Invalid fieldmap.');
      return;
    }
    // Load the fieldmap.
    $context['sandbox']['salesforce']['map'] = $map;

    // Load the object definitions.
    $context['sandbox']['salesforce']['drupal_object'] =
      salesforce_api_fieldmap_objects_load('drupal', $map->drupal);
    // $context['sandbox']['salesforce']['salesforce_object'] =
    //   salesforce_api_fieldmap_objects_load('salesforce', $map['salesforce']);
    $sql = 'SELECT oid, sfid FROM {salesforce_object_map} WHERE name = "%s"';
    $result = db_query($sql, $map->name);
    while ($row = db_fetch_array($result)) {
      $context['sandbox']['salesforce']['existing'][$row['sfid']] = $row['oid'];
    }

    $soql = 'SELECT '
      // "Id" must be included in the SOQL query.
      . implode(', ', array_keys($map->fields + array('Id' => '')))
      . ' FROM ' . $map->salesforce;
    if (!empty($extra['extra-where'])) {
      $soql .= ' WHERE ' . $extra['extra-where'];
    }
    try {
      $sf = salesforce_api_connect();
        $context['sandbox']['salesforce']['query'] = $query = $sf->client->query($soql);
      } catch (Exception $e) {
      $context['finished'] = 1;
      $context['message'] = $e->getMessage();
        return;
    }
    if (empty($query->records)) {
        $context['finished'] = 1;
      $context['message'] = 'Empty resultset returned from Salesforce query.';
    }
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['position'] = 0;
    $context['sandbox']['imported'] = 0;
    $context['finished'] = 0;
  }

  $map = $context['sandbox']['salesforce']['map'];
  $query = $context['sandbox']['salesforce']['query'];
  if (array_key_exists('existing', $context['sandbox']['salesforce'])) {
      $existing = $context['sandbox']['salesforce']['existing'];
  }
  else {
      $existing = '';
  }
  $drupal_object = $context['sandbox']['salesforce']['drupal_object'];
  $pos = $context['sandbox']['position'];
  $query_array = get_object_vars($query);
  $size = $query_array['size'];

  if (!$query_array['done'] && !empty($query_array['queryLocator']) && !isset($query_array['records'][$pos])) {
    try {
        $sf = salesforce_api_connect();
        $context['sandbox']['salesforce']['query'] = $query = $sf->client->queryMore($query_array['queryLocator']);
        $query_array = get_object_vars($query);
        $context['sandbox']['position'] = $pos = key($query_array['records']);
    } catch (Exception $e) {
      $context['finished'] = 1;
      $context['message'] = $e->getMessage();
      return;
    }
  }

  // Do ONE record at a time so we don't go over the max execution limit.
  $record = $query_array['records'][$pos];
  if ($context['sandbox']['progress'] >= $size || empty($record)) {
    $context['finished'] = 1;
    $context['message'] = 'Imported ' . $context['sandbox']['imported'] . ' SalesForce records.';
    return;
  }
  // For some reason, writing the SObject to session data destroys the object.
  // Cast it to an array to recover the corrected object.
  if (!is_array($record)) {
    $record = get_object_vars($record);
  }
  $created = !isset($existing[$record['Id']]);
  $type = $map->drupal;
  // "node" mappings are like "node_contenttype".
  // others are like "user", "uc_order", etc.
  if (strpos($type, 'node_') === 0) {
    $type = 'node';
  }

  $function = 'sf_' . $type . '_import';

  if (function_exists($function)) {
    $oid = $function($record, $map->name, $existing[$record['Id']], $extra);
  }
  else {
    $context['finished'] = TRUE;
    $context['success'] = FALSE;
    $context['results'][] = 'Could not find import function ' . $function;
  }

  if ($oid) {
    $context['results'][] = ($created ? 'Created' : 'Updated') . ' ' . l($type . ' ' . $oid, $type . '/' . $oid);
    $context['sandbox']['imported']++;
  }

  $context['sandbox']['progress']++;
  $context['sandbox']['position']++;
  $context['finished'] = $context['sandbox']['progress'] / $size;
}

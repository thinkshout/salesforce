<?php

/**
 * Display a list of available Salesforce Mappings.
 *
 * @return array
 */
function salesforce_mappings_page() {
  $mappings = salesforce_mapping_load();
  foreach ($mappings as $mapping) {
    $uri = entity_uri('salesforce_mapping', $mapping);
    $rows[] = array(
      l($mapping->label, $uri['path'] . '/edit'),
      $mapping->drupal_entity_type,
      $mapping->drupal_bundle,
      $mapping->salesforce_object_type,
      $mapping->salesforce_record_type,
    );
  }

  if (empty($rows)) {
    $rows[] = array(
      array(
        'data' => t('There are no Salesforce mappings.'),
        'colspan' => 5
      )
    );
  }

  $header = array(
    t('Label'),
    t('Drupal entity'),
    t('Drupal bundle'),
    t('Salesforce object'),
    t('Salesforce record type'),
  );

  $table = array(
    '#theme' => 'table',
    '#rows' => $rows,
    '#header' => $header
  );

  return $table;
}

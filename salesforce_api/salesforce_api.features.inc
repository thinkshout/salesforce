<?php
// $Id$

/**
 * @file
 * Salesforce features integration
 */

/**
 * @defgroup {
 * Salesforce Field Map features integration
 */

/**
 * Implements hook_features_export_options().
 */
function salesforce_field_map_features_export_options() {
  $maps = salesforce_api_salesforce_field_map_load_all();
  $options = array();
  foreach ($maps as $machine_name => $map) {
    $options[$machine_name] = $map->drupal . ' => ' . $map->salesforce;
  }
  return $options;
}

/**
 * Implements hook_features_export().
 */
function salesforce_field_map_features_export($data, &$export, $module_name = 'salesforce_api') {
  // Export node type entries from {salesforce_field_map}.
  return ctools_component_features_export('salesforce_field_map', $data, $export, $module_name);
}

/**
 * Implements hook_features_export_render().
 */
function salesforce_field_map_features_export_render($module, $data, $export = NULL) {
  return ctools_component_features_export_render('salesforce_field_map', 'salesforce_api', $data);
}

/**
 * Implements hook_features_revert().
 */
function salesforce_field_map_features_revert($module = 'salesforce_api') {
  ctools_component_features_revert($module);
}

/**
 * }
 */

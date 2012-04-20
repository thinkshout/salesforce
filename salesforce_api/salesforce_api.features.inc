<?php
// $Id$

/**
 * @file
 * Salesforce features integration
 */

/**
 * @defgroup {
 * Salesforce Fieldmap features integration functions
 */

/**
 * Implements hook_features_export_options().
 */
function salesforce_api_fieldmap_features_export_options() {
  $maps = salesforce_api_salesforce_fieldmap_load_all();
  $options = array();
  foreach ($maps as $machine_name => $map) {
    $options[$machine_name] = $map->drupal_entity . ' : ' . $map->drupal_bundle . ' => ' . $map->salesforce;
  }
  return $options;
}

/**
 * Implements hook_features_export().
 */
function salesforce_api_fieldmap_features_export($data, &$export, $module_name = 'salesforce_api') {
  // Export mappings from {salesforce_fieldmap}.
  return ctools_component_features_export('salesforce_fieldmap', $data, $export, $module_name);
}

/**
 * Implements hook_features_export_render().
 */
function salesforce_api_fieldmap_features_export_render($module, $data, $export = NULL) {
  return ctools_component_features_export_render('salesforce_fieldmap', 'salesforce_api', $data);
}

/**
 * Implements hook_features_revert().
 */
function salesforce_api_fieldmap_features_revert($module = 'salesforce_api') {
  ctools_component_features_revert($module);
}

/**
 * }
 */

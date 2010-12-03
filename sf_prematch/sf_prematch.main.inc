<?php
// $Id$

/**
 * @file
 * Import/Export functions for sf_prematch module.
 */

/**
 *  Use prematch rule to find a salesforce object to match the node.
 *
 * @param string $object_type
 * @param stdObject $drupal_object
 * @param array $map
 * @param array $match_by
 * @return string sfid
 */
function sf_prematch_export($object_type, $drupal_object, $map, $match_by) {
  if (empty($drupal_object) || empty($map->drupal) || empty($match_by)) {
    return;
  }
  // Get information to allow using handlers to get values of Drupal fields.
  $drupal_object_info = salesforce_api_fieldmap_objects_load('drupal', $map->drupal);
  $drupal_fields_info = $drupal_object_info['fields'];
  $sf_object_info = salesforce_api_fieldmap_objects_load('salesforce', $map->salesforce);

  $values = array();
  $select_clause = "SELECT id";
  $safe_fields = array();
  foreach ($map->fields as $key => $value) {
    if (is_array($value)) { continue; }
    $safe_fields[$key] = $value;
  }
  $map->fields = $safe_fields;
  $drupal_to_salesforce_fieldmap = array_flip($map->fields);

  // Create $values array allowing easy mapping from match_by fields to query fields and values.
  // Also build out SOQL select clause to include all match by sf fields.
  foreach ($match_by as $match_by_field => $drupal_field_name) {
    if ($match_by_field == 'fieldmap' || $match_by_field == 'rule') {continue;}
    $values[$match_by_field] = array();
    // If a handler is specified for retrieving a value for the source field...
    if (isset($drupal_fields_info[$drupal_field_name]['export'])) {
      $drupal_field_definition = $drupal_fields_info[$drupal_field_name];
      $sf_field_definition = $sf_object_info['fields'][$drupal_to_salesforce_fieldmap[$drupal_field_name]];
      // Get the value for the field from the handler function.
      $function = $drupal_fields_info[$drupal_field_name]['export'];
      $drupal_value = $function($drupal_object, $drupal_field_name, $drupal_field_definition, $sf_field_definition);
    }
    // Otherwise set the field on the export object to the value of the source
    // field if it's present on the source object.
    elseif (isset($drupal_object->$drupal_field_name)) {
      $drupal_value = $drupal_object->$drupal_field_name;
    }
    else {
    	$drupal_value = null;
    }
    $values[$match_by_field]['drupal_value'] = $drupal_value;
    $values[$match_by_field]['salesforce_field_name'] = $drupal_to_salesforce_fieldmap[$drupal_field_name];

    // If there's a salesforce field to match by, include it in the select clause.
    if ($values[$match_by_field]['salesforce_field_name']) {
    	$select_clause .= ', ' . $values[$match_by_field]['salesforce_field_name'];
    }
  }

  $sf_class = $map->salesforce;
  $from_where_clause = " FROM $sf_class WHERE ";

  // Use match by rule to build out where clause.
  switch ($match_by['rule']) {
  	case SF_PREMATCH_PRIMARY_SECONDARY_AND_TERTIARY:
  		$from_where_clause .= $values['tertiary_field']['salesforce_field_name'] . " = '" .
  		  $values['tertiary_field']['drupal_value'] . "' AND ";
  	// no break;
  	case SF_PREMATCH_PRIMARY_AND_SECONDARY:
  		$from_where_clause .= $values['secondary_field']['salesforce_field_name'] . " = '" .
  		  $values['secondary_field']['drupal_value'] . "' AND ";
  	// no break;
  	case SF_PREMATCH_PRIMARY:
  		$from_where_clause .= $values['primary_field']['salesforce_field_name'] . " = '" .
  		  $values['primary_field']['drupal_value'] . "'";
  	break;
  	default:
  	  return;
  	break;
  }

  $query = $select_clause . $from_where_clause;

  // Run the SOQL query against the Salesforce API
  if ($sf = salesforce_api_connect()) {
    try {
      $result = $sf->client->query($query);
    }
    catch (Exception $e) {
      DrupalSalesforce::watchdog(SALESFORCE_LOG_SOME, 'Exception in sf_prematch stage: ' . $e->getMessage(), array(), WATCHDOG_ALERT);
      return;
    }

    switch ($result->size) {
    	case 0:
    		return;
    	break;
    	case 1:
    		return $result->records[0]->Id;
    	break;
    	default:
    	  // TODO: handle multiple records
    	break;
    }
  }
}

function sf_prematch_import($object_type, $object, $map, $match_by) {
  // As of 2/23/2010 there is no out of the box functionality to import a salesforce
  // record without already having established a mapping. Therefore, developers
  // exposing such functionality should implement their own hook_sf_find_map:import
  return;
}

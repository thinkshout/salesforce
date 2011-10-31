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
  $safe_fields = array();
  foreach ($map->fields as $key => $value) {
    if (is_array($value)) {
      continue;
    }
    $safe_fields[$key] = $value;
  }
  $map->fields = $safe_fields + array('Id' => '');
  $drupal_to_salesforce_fieldmap = array_flip($map->fields);

  // Create $values array allowing easy mapping from match_by fields to query fields and values.
  // Also build out SOQL select clause to include all match by sf fields.
  foreach ($match_by as $match_by_field => $drupal_field_name) {
    if ($match_by_field == 'fieldmap' || $match_by_field == 'rule' || !$drupal_to_salesforce_fieldmap[$drupal_field_name]) {
      continue;
    }

    // If a handler is specified for retrieving a value for the source field...
    $drupal_value = NULL;
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

    if (!empty($drupal_value)) {
      $values[$match_by_field]['drupal_value'] = $drupal_value;
      $values[$match_by_field]['salesforce_field_name'] = $drupal_to_salesforce_fieldmap[$drupal_field_name];
      $values[$match_by_field]['salesforce_type'] = $sf_field_definition['salesforce']['type'];
    }
  }

  // If no values were found for the fields found, then we have nothing to match on
  if (empty($values)) {
    return;
  }

  foreach ($values as $key => $value) {
    switch ($value['salesforce_type']) {
      case 'boolean':
      case 'time':
      case 'date':
      case 'datetime':
      case 'percent':
      case 'currency':
      case 'int':
      case 'double':
        // no quotes around SOQL field expressions for dates and numbers
        break;
      default:
        // single quotes around SOQL field expressions for all others
        $values[$key]['drupal_value'] =
          "'" . $values[$key]['drupal_value'] . "'";
        break;
    }
  }

  // Use match by rule to build out where clause.
  $where_clause = '';
  switch ($match_by['rule']) {
    case SF_PREMATCH_PRIMARY_SECONDARY_AND_TERTIARY:
      $where_clause .= $values['tertiary_field']['salesforce_field_name'] . " = " . $values['tertiary_field']['drupal_value'] . " AND ";
      // no break;
    case SF_PREMATCH_PRIMARY_AND_SECONDARY:
      $where_clause .= $values['secondary_field']['salesforce_field_name'] . " = " . $values['secondary_field']['drupal_value'] . " AND ";
      // no break;
    case SF_PREMATCH_PRIMARY:
      $where_clause .= $values['primary_field']['salesforce_field_name'] . " = " . $values['primary_field']['drupal_value'];
      break;
    default:
      // no prematch is set
      return;
  }

  $query = 'SELECT Id FROM ' . $map->salesforce . ' WHERE ' . $where_clause . ' ORDER BY CreatedDate ASC';

  // Run the SOQL query against the Salesforce API
  if ($sf = salesforce_api_connect()) {
    try {
      $result = $sf->client->query($query);
    }
    catch (Exception $e) {
      salesforce_api_log(SALESFORCE_LOG_SOME, 'Exception in sf_prematch stage: ' . $e->getMessage(), array(), WATCHDOG_ALERT);
      return;
    }

    switch ($result->size) {
      case 0:
      return;
      case 1:
      default:
        return $result->records[0]->Id;
        break;
    }
  }
}

/**
 *  Use prematch rule to find a drupal node to match the SF object.
 *
 * @param string $object_type ("user" or "node" currently defined)
 * @param stdObject $object
 * @param array $map
 * @param array $match_by
 *
 * @return string uid or nid
 */

function sf_prematch_import($object_type, $object, $map, $match_by) {

  // This is a generic solution only for object types of "user" or "node".
  // If another object type is found, no prematching will be done.
  // The developer should implement their own hook_sf_find_match() function
  // to match this unsupportted type.
  switch ($object_type) {
    case "user":
      $id_name = "uid";
      $table_name = "users";
      $fields_table_name = 'profile_values';
      $schema = drupal_get_schema($table_name);
      $fields_schema = _sf_prematch_get_user_schema();
      $fields_schema_field_suffix = '';
      break;
    case "node":
      $id_name = "nid";
      $table_name = "node";
      $content_type = str_replace("node_", "", $map->drupal);
      $schema = drupal_get_schema($table_name);
      $fields_table_name = 'content_type_' . $content_type;
      $fields_schema = drupal_get_schema($fields_table_name);
      $fields_schema_field_suffix = "_value";
      break;
    default:
      // Undefined object type
      return;
      break;
  }

  $primary_field   = $match_by['primary_field']   <> '0' ? $match_by['primary_field']   : '';
  $secondary_field = $match_by['secondary_field'] <> '0' ? $match_by['secondary_field'] : '';
  $tertiary_field  = $match_by['tertiary_field']  <> '0' ? $match_by['tertiary_field']  : '';
  $fields_table_used = FALSE;
  $where = '';
  $joins = '';

  // Process primary_field
  if (!empty($primary_field)) {
    // Look for field in fields table
    $sf_field_name = array_search($primary_field, $map->fields);
    if (isset($fields_schema['fields'][$primary_field . $fields_schema_field_suffix])) {
      if ($object_type == 'node') {
        $where .= sprintf("%s.%s = '%s'", $fields_table_name, $primary_field . $fields_schema_field_suffix, $object->$sf_field_name);
      }
      if ($object_type == 'user') {
        $where .= sprintf("%s_p.fid = '%s' AND %s_p.value = '%s'",
          $fields_table_name,
          $fields_schema['fields'][$primary_field],
          $fields_table_name,
          $object->$sf_field_name);
        $joins .= sprintf("INNER JOIN {%s} %s_p ", $fields_table_name, $fields_table_name);
      }
      $fields_table_used = TRUE;
    }
    else {
      // Look for field in main table
      if (isset($schema['fields'][$primary_field])) {
        $where .= sprintf("%s.%s = '%s'", $table_name, $primary_field, $object->$primary_field);
      }
      else {
        // The match_by field is in neither table
        drupal_set_message(t('Primary field [@primary_field] specified in map prematch not defined.', array('@primary_field' => $primary_field)));
        return;
      }
    }

    // Process secondary_field
    if (!empty($secondary_field)) {
      // Look for field in fields table
      $sf_field_name = array_search($secondary_field, $map->fields);
      if (isset($fields_schema['fields'][$secondary_field . $fields_schema_field_suffix])) {
        if ($object_type == 'node') {
          $where .= sprintf(" AND %s.%s = '%s'", $fields_table_name, $secondary_field . $fields_schema_field_suffix, $object->$sf_field_name);
        }
        if ($object_type == 'user') {
          $where .= sprintf(" AND %s_s.fid = '%s' AND %s_s.value = '%s'",
            $fields_table_name,
            $fields_schema['fields'][$secondary_field],
            $fields_table_name,
            $object->$sf_field_name);
          $joins .= sprintf("INNER JOIN {%s} %s_s ", $fields_table_name, $fields_table_name);
        }
        $fields_table_used = TRUE;
      }
      else {
        // Look for field in main table
        if (isset($schema['fields'][$secondary_field])) {
          $where .= sprintf(" AND %s.%s = '%s'", $table_name, $secondary_field, $object->$secondary_field);
        }
        else {
          // The match_by field is in neither table
          drupal_set_message(t('Secondary field [@secondary_field] specified in map prematch not defined.', array('@secondary_field' => $secondary_field)));
          return;
        }
      }

      // Process tertiary_field
      if (!empty($tertiary_field)) {
        // Look for field in fields table
        $sf_field_name = array_search($tertiary_field, $map->fields);
        if (isset($fields_schema['fields'][$tertiary_field . $fields_schema_field_suffix])) {
          if ($object_type == 'node') {
            $where .= sprintf(" AND %s.%s = '%s'", $fields_table_name, $tertiary_field . $fields_schema_field_suffix, $object->$sf_field_name);
          }
          if ($object_type == 'user') {
            $where .= sprintf(" AND %s_t.fid = '%s' AND %s_t.value = '%s'",
              $fields_table_name,
              $fields_schema['fields'][$tertiary_field],
              $fields_table_name,
              $object->$sf_field_name);
            $joins .= sprintf("INNER JOIN {%s} %s_t ", $fields_table_name, $fields_table_name);
          }
          $fields_table_used = TRUE;
        }
        else {
          // Look for field in main table
          if (isset($schema['fields'][$tertiary_field])) {
            $where .= sprintf(" AND %s.%s = '%s'", $table_name, $tertiary_field, $object->$tertiary_field);
          }
          else {
            // The match_by field is in neither table
            drupal_set_message(t('Tertiary field [@tertiary_field] specified in map prematch not defined.'), array('@tertiary_field' => $tertiary_field));
            return;
          }
        }
      } // end of tertiary_field
    } // end of secondary_field
  } // end of primary_field


  $query_str = sprintf("SELECT %s.%s FROM {%s} ", $table_name, $id_name, $table_name);

  if ($fields_table_used == TRUE && $object_type == 'node') {
    $query_str .= sprintf("INNER JOIN {%s} ", $fields_table_name);
    $where .= sprintf(" AND %s.%s=%s.%s", $table_name, $id_name, $fields_table_name, $id_name);
  }

  if ($fields_table_used == TRUE && $object_type == 'user') {
    $query_str .= $joins;
    $where .= sprintf(" AND %s.%s=%s_p.%s", $table_name, $id_name, $fields_table_name, $id_name);
  }

  $query_str .= sprintf("WHERE %s", $where);

  if (empty($where)) {
    // We shouldn't ever get here, but just in case...
    return;
  }

  // Now that we have constructed the query string, execute it and store the results.
  $found = array();
  if (strlen($query_str) > 0) {
    $results = db_query($query_str);
    while ($f = db_result($results)) {
      $found[] = $f;
    }
  }

  if (count($found) > 0) {
    // Return only the first match. Someday we may want to handle multiple matches.
    return reset($found);
  }
  else {
    // Nothing found so return nothing
    return;
  }
}

/**
 * Returns an array of the profile fields formatted like Drupal's schema
 */
function _sf_prematch_get_user_schema() {
  $schema = array();

  if (module_exists('profile')) {
    $results = db_query("SELECT name, fid FROM {profile_fields}");
    while ($result = db_fetch_array($results)) {
      $schema['fields'][$result['name']] = $result['fid'];
    }
  }

  return $schema;
}

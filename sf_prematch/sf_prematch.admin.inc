<?php
// $Id$

/**
 * @file
 * Admin functions for sf_prematch module.
 */

function sf_prematch_list() {
  // Define the header for the admin table.
  $header = array(
    t('Drupal object'),
    t('Salesforce object'),
    array('data' => t('Operations'), 'colspan' => 3),
  );

  //$maps = salesforce_api_salesforce_field_map_load_all();
  $result = db_query("SELECT fm.*, pm.rule " .
    "FROM {salesforce_field_map} fm LEFT JOIN {salesforce_prematch} pm " .
    "ON fm.name = pm.name"
  );
  $rows = array();

  // Loop through all the indexed field maps.
  while ($map = db_fetch_object($result)) {
    if ($map->rule) {
      $op_add_edit =  l(t('edit prematch'), SALESFORCE_PATH_FIELDMAPS . '/' .  $map->name . '/prematching');
      $op_del = l(t('delete prematch'), SALESFORCE_PATH_FIELDMAPS . '/' . $map->name . '/prematching/delete');
    }
    else {
      $op_add_edit =  l(t('add prematch'), SALESFORCE_PATH_FIELDMAPS . '/' . $map->name . '/prematching');
      $op_del = '';
    }
    // Add the row to the table with the basic operations.
    $rows[] = array(
      salesforce_api_fieldmap_object_label('drupal', $map->drupal),
      salesforce_api_fieldmap_object_label('salesforce', $map->salesforce),
      $op_add_edit,
      $op_del,
    );
  }

  // Add a message if no objects have been mapped.
  if (empty($rows)) {
    $rows[] = array(
      array('data' => t('You have not yet assigned prematching to any fieldmaps.'), 'colspan' => 7),
    );
  }

  $output = theme('table', $header, $rows);
  return $output;
}

// Displays the form to add prematching to a fieldmap.
function sf_prematch_edit_form(&$form_state, $fieldmap) {
  // Load the fieldmap from the database.
  $map = salesforce_api_fieldmap_load($fieldmap);

  // Return to the admin page if the fieldmap did not exist.
  if (empty($map)) {
    drupal_set_message(t('That fieldmap does not exist.'), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  }

  // Return to the admin page if the fieldmap has no mapped fields.
  if (empty($map->fields)) {
    drupal_set_message(t('That fieldmap exists, but does not have any fields.'), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  }

  // Load the prematch from the database.
  $prematch = sf_prematch_match_by_load($fieldmap);

  $form = array();

  // Add the index to the form array.
  $form['name'] = array(
    '#type' => 'value',
    '#value' => $fieldmap,
  );

  // Set flag for use in deciding where to redirect to on form submit.
  $form['new_prematch'] = array(
    '#type' => 'value',
    '#value' => ($prematch['primary_field'] == '' ||
      (isset($form_state['values']['new_prematch']) && $form_state['values']['new_prematch'])),
  );

  // Add a description of the prematch to the form array.
  $form['prematch_desc'] = array(
    '#value' => '<p>' . t('Before creating a new object, attempt to match an existing one using the fields and rules below. (Click cancel to skip this step.)') . '</p>',
  );

  // Add the select lists for the mapped Drupal field(s) to use in prematching.
  $form['primary'] = array(
    '#type' => 'select',
    '#title' => t('Primary Field'),
    '#options' => sf_prematch_get_options($map, TRUE),
    '#default_value' => $prematch['primary_field'],
    '#required' => TRUE,
  );

  $options = sf_prematch_get_options($map);

  $form['secondary'] = array(
    '#type' => 'select',
    '#title' => t('Secondary Field'),
    '#options' => $options,
    '#default_value' => $prematch['secondary_field'],
    '#required' => FALSE,
  );
  $form['tertiary'] = array(
    '#type' => 'select',
    '#title' => t('Tertiary Field'),
    '#options' => $options,
    '#default_value' => $prematch['tertiary_field'],
    '#required' => FALSE,
  );

  // Create options to use in rule select, primary only first
  $options = array(
    SF_PREMATCH_PRIMARY => 'primary field',
    SF_PREMATCH_PRIMARY_AND_SECONDARY => 'primary and secondary fields',
    SF_PREMATCH_PRIMARY_SECONDARY_AND_TERTIARY => 'primary, secondary and tertiary fields',
  );

  // Add the select list for prematching rule.
  $form['rule'] = array(
    '#type' => 'select',
    '#title' => t('Only consider a match if the found object matches'),
    '#options' => $options,
    '#default_value' => $prematch['rule'],
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => l(t('Cancel'), SALESFORCE_PATH_FIELDMAPS . '/prematching'),
  );

  return $form;
}

function sf_prematch_edit_form_validate($form, &$form_state) {

}

function sf_prematch_edit_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  // Store data in the database.
  if ($values['new_prematch']) {
    db_query("INSERT INTO {salesforce_prematch} (name, primary_field, secondary_field, tertiary_field, rule) VALUES ('%s', '%s', '%s', '%s', %d)", $values['name'], $values['primary'], $values['secondary'], $values['tertiary'], $values['rule']);
  }
  else {
    db_query("UPDATE {salesforce_prematch} SET primary_field = '%s', secondary_field = '%s', tertiary_field = '%s', rule = %d WHERE name = '%s'", $values['primary'], $values['secondary'], $values['tertiary'], $values['rule'], $values['name']);
  }

  // Display a message.
  drupal_set_message(t('The changes have been saved.'));

  // Redirect to fieldmap list or prematching page.
  $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS;
}

function sf_prematch_delete_form(&$form_state, $fieldmap) {
  // Load the fieldmap and prematch from the database.
  $map = salesforce_api_fieldmap_load($fieldmap);
  $match_by = sf_prematch_match_by_load($fieldmap);

  // Return to the admin page if the fieldmap did not exist.
  if (empty($match_by)) {
    drupal_set_message(t('That prematch does not exist.'), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS . '/prematching');
  }

  $form = array();

  // Add the fieldmap to the form array.
  $form['name'] = array(
    '#type' => 'value',
    '#value' => $fieldmap,
  );

  // Build the description text for this prematch.
  $desc = t('You are about to delete the prematch for fieldmap ');
  if ($map->action == 'import') {
    $desc .= t('Maps Salesforce %salesforce objects to Drupal %drupal objects for import.', array('%drupal' => $map->drupal, '%salesforce' => $map->salesforce));
  }
  else {
    $desc .= t('That fieldmap maps Drupal %drupal objects to Salesforce %salesforce objects for export.', array('%drupal' => $map->drupal, '%salesforce' => $map->salesforce));
  }

  return confirm_form($form, t('Are you sure you want to delete this prematch?'), SALESFORCE_PATH_FIELDMAPS . '/prematching', $desc, t('Delete'));
}

function sf_prematch_delete_form_submit($form, &$form_state) {
  // Delete the specified prematch.
  sf_prematch_match_by_delete($form_state['values']['name']);

  // Display a message and return to the admin prematch screen.
  drupal_set_message(t('The prematch has been deleted.'));

  $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS . '/prematching';
}

function sf_prematch_get_options($map, $required = FALSE) {
  // Extract terms from $map.
  if ($map->actions == 'import') {
    $terms = array_keys($map->fields);
  }
  else {
    $terms = array_values($map->fields);
  }
  sort($terms);

  // Build terms into ordered options to use in select.
  $options = array();
  // Start with empty option if select is not required.
  if (!$required) {
    $options[] = '';
  }

  // Add terms to options, making key = value so form value is key not integer.
  foreach ($terms as $term) {
    if (is_array($term)) {
      $term = implode(' : ', $term);
      if (strlen($term) > 50) {
        $term = substr($term, 0, 50) . ' ...';
      }
    }
    $options[$term] = $term;
  }

  return $options;
}

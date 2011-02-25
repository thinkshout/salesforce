<?php

/**
 * @file
 * Admin functions for sf_prematch module.
 */

function sf_prematch_list() {
  // Define the header for the admin table.
  $header = array(
    t('Index'),
    t('Drupal object'),
    t('Salesforce object'),
    array('data' => t('Operations'), 'colspan' => 3),
  );

  $result = db_query("SELECT salesforce_field_map.*, salesforce_prematch.rule " .
    "FROM salesforce_field_map LEFT JOIN salesforce_prematch " .
    "ON salesforce_field_map.fieldmap = salesforce_prematch.fieldmap"
  );
  $rows = array();

  // Loop through all the indexed field maps.
  while ($map = db_fetch_array($result)) {
    if ($map['rule']) {
    	$op_0 =  l(t('edit prematch'), SALESFORCE_PATH_FIELDMAPS . '/prematching/' . $map['fieldmap']);
    	$op1 = l(t('delete prematch'), SALESFORCE_PATH_FIELDMAPS . '/prematching/' . $map['fieldmap'] . '/delete');
    }
    else {
    	$op_0 =  l(t('add prematch'), SALESFORCE_PATH_FIELDMAPS . '/prematching/' . $map['fieldmap']);
    	$op1 = '';
    }
    // Add the row to the table with the basic operations.
    $rows[] = array(
      $map['fieldmap'],
      salesforce_api_fieldmap_object_label('drupal', $map['drupal']),
      salesforce_api_fieldmap_object_label('salesforce', $map['salesforce']),
      $op_0,
      $op1,
    );
  }

  // Add a message if no objects have been mapped.
  if (count($rows) == 0) {
    $rows[] = array(
      array('data' => t('You have not yet defined any fieldmaps.'), 'colspan' => 7),
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
  if (empty($map['fields'])) {
    drupal_set_message(t('That fieldmap exists, but does not have any fields.'), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  }

  // Load the prematch from the database.
  $prematch = sf_prematch_match_by_load($fieldmap);

  $form = array();

  // Add the index to the form array.
  $form['fieldmap_index'] = array(
    '#type' => 'value',
    '#value' => $fieldmap,
  );

  // Set flag for use in deciding where to redirect to on form submit.
  $is_new =
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
    '#options' => sf_prematch_get_options($map, true),
    '#default_value' => $prematch['primary_field'],
    '#required' => true,
  );

  $options = sf_prematch_get_options($map);

  $form['secondary'] = array(
    '#type' => 'select',
    '#title' => t('Secondary Field'),
    '#options' => $options,
    '#default_value' => $prematch['secondary_field'],
    '#required' => false,
  );
  $form['tertiary'] = array(
    '#type' => 'select',
    '#title' => t('Tertiary Field'),
    '#options' => $options,
    '#default_value' => $prematch['tertiary_field'],
    '#required' => false,
  );

  // Create options to use in rule select.
  $options = array(
    SF_PREMATCH_PRIMARY_SECONDARY_AND_TERTIARY => 'primary, secondary and tertiary fields',
    SF_PREMATCH_PRIMARY_AND_SECONDARY => 'primary and secondary fields',
    SF_PREMATCH_PRIMARY => 'primary field',
  );

  // Add the select list for prematching rule.
  $form['rule'] = array(
    '#type' => 'select',
    '#title' => t('Only consider a match if the found object matches'),
    '#options' => $options,
    '#default_value' => $prematch['rule'],
    '#required' => true,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => l(t('Cancel'), SALESFORCE_PATH_FIELDMAPS),
  );

  return $form;
}

function sf_prematch_edit_form_validate($form, &$form_state) {

}

function sf_prematch_edit_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  // Store data in the database.
  if ($values['new_prematch']) {
    db_query("INSERT INTO {salesforce_prematch} (fieldmap, primary_field, secondary_field, tertiary_field, rule) VALUES (%d, '%s', '%s', '%s', %d)", $values['fieldmap_index'], $values['primary'], $values['secondary'], $values['tertiary'], $values['rule']);
  }
  else {
  	db_query("UPDATE {salesforce_prematch} SET primary_field = '%s', secondary_field = '%s', tertiary_field = '%s', rule = %d WHERE fieldmap = %d", $values['primary'], $values['secondary'], $values['tertiary'], $values['rule'], $values['fieldmap_index']);
  }

  // Display a message.
  drupal_set_message(t('The changes have been saved.'));

  // Redirect to fieldmap list or prematching page.
  if ($values['new_prematch']) {
  	$form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS;
  }
  else {
  	$form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS . '/prematching';
  }
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
  $form['fieldmap_index'] = array(
    '#type' => 'value',
    '#value' => $fieldmap,
  );

  // Build the description text for this prematch.
  $desc = t('You are about to delete the prematch for fieldmap ');
  if ($map['action'] == 'import') {
    $desc .= t('@index. That fieldmap maps Salesforce %salesforce objects to Drupal %drupal objects for import.', array('@index' => $map['fieldmap'], '%drupal' => $map['drupal'], '%salesforce' => $map['salesforce']));
  }
  else {
    $desc .= t('@index. That fieldmap maps Drupal %drupal objects to Salesforce %salesforce objects for export.', array('@index' => $map['fieldmap'], '%drupal' => $map['drupal'], '%salesforce' => $map['salesforce']));
  }

  return confirm_form($form, t('Are you sure you want to delete this prematch?'), SALESFORCE_PATH_FIELDMAPS . '/prematching', $desc, t('Delete'));
}

function sf_prematch_delete_form_submit($form, &$form_state) {
  // Delete the specified prematch.
  sf_prematch_match_by_delete($form_state['values']['fieldmap_index']);

  // Display a message and return to the admin prematch screen.
  drupal_set_message(t('The prematch has been deleted.'));

  $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS . '/prematching';
}

function sf_prematch_get_options($map, $required = false) {
  // Extract terms from $map.
  if ($map['action'] == 'import') {
  	$terms = array_keys($map['fields']);
  }
  else {
  	$terms = array_values($map['fields']);
  }
  sort($terms);

  // Build terms into ordered options to use in select.
  $options = array();
  // Start with empty option if select is not required.
  if (!$required) {$options[] = '';}

  // Add terms to options, making key = value so form value is key not integer.
  foreach ($terms as $term) {
  	$options[$term] = $term;
  }

  return $options;
}

<?php
// $Id$

/**
 * @file
 * Contains the admin page callbacks for the Salesforce module, including forms
 *   for general settings and fieldmap administration.
 */

// Settings form submit handler so that password doesn't get deleted.
function salesforce_api_settings_form_submit($form, &$form_state) {
  // If the user hit "Save Configuration" and the required field
  // salesforce_api_password is blank, try to get it from variables
  if ($form_state['values']['submit'] == $form_state['values']['op'] and empty($form_state['values']['salesforce_api_password'])) {
    $pass = variable_get('salesforce_api_password', FALSE);
    if (isset($pass)) {
      $form_state['values']['salesforce_api_password'] = $pass;
    }
  }
}

// Displays an admin table for fieldmaps.
function salesforce_api_fieldmap_admin() {
  // Define the header for the admin table.
  $header = array(t('Index'), t('Drupal object'), t('Salesforce object'), array('data' => t('Operations'), 'colspan' => 3));
  $rows = array();

  // Loop through all the indexed field maps.
  for ($i = 1; $i <= variable_get('salesforce_fieldmap_index', 0); $i++) {
    // Load the map for this index.
    $map = variable_get('salesforce_fieldmap_'. $i, array());

    // If it actually existed...
    if (!empty($map)) {
      // Add the row to the table with the basic operations.
      $rows[] = array(
        $map['index'],
        salesforce_api_fieldmap_object_label('drupal', $map['drupal']),
        salesforce_api_fieldmap_object_label('salesforce', $map['salesforce']),
        l(t('edit'), SALESFORCE_PATH_FIELDMAPS .'/'. $map['index'] .'/edit'),
        l(t('delete'), SALESFORCE_PATH_FIELDMAPS .'/'. $map['index'] .'/delete'),
      );
    }
  }

  // Add a message if no objects have been mapped.
  if (count($rows) == 0) {
    $rows[] = array(
      array('data' => t('You have not yet defined any fieldmaps.'), 'colspan' => 7),
    );
  }

  return theme('table', $header, $rows);
}

// Displays the form to add a fieldmap.
function salesforce_api_fieldmap_add_form(&$form_state) {
  $form = array();

  // Build an options array out of the Drupal objects.
  $options = array();

  foreach (salesforce_api_fieldmap_objects_load('drupal') as $key => $value) {
    $options[$key] = $value['label'];
  }

  $form['drupal_object'] = array(
    '#type' => 'select',
    '#title' => t('Drupal object'),
    '#options' => count($options) > 0 ? $options : array(t('None available')),
    '#disabled' => count($options) == 0,
    '#required' => TRUE,
  );

  // Build an options array out of the Salesforce objects.
  $options = array();

  foreach (salesforce_api_fieldmap_objects_load('salesforce') as $key => $value) {
    $options[$key] = $value['label'];
  }

  $form['salesforce_object'] = array(
    '#type' => 'select',
    '#title' => t('Salesforce object'),
    '#options' => count($options) > 0 ? $options : array(t('None available')),
    '#disabled' => count($options) == 0,
    '#required' => TRUE,
  );
  $form['automatic'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically Populate Salesforce?'),
    '#return_value' => 1,
    '#default_value' => 1,
    '#description' => t('Automatically create and link new salesforce objects when Drupal objects are created?'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Map object fields'),
    '#suffix' => l(t('Cancel'), SALESFORCE_PATH_FIELDMAPS),
  );

  return $form;
}

// Submit handler for a new fieldmap
function salesforce_api_fieldmap_add_form_submit($form, &$form_state) {
  // Create the new fieldmap.
  $index = salesforce_api_fieldmap_create($form_state['values']['drupal_object'], $form_state['values']['salesforce_object'], $form_state['values']['automatic']);

  // Redirect to its edit form.
  $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS .'/'. $index .'/edit';
}


/**
 * Creates a new fieldmap in the database and returns its index.
 *
 * @param $drupal
 *   The name of a Drupal object.
 * @param $salesforce
 *   The name of a Salesforce object.
 * @return
 *   The numeric index of the new fieldmap.
 */
function salesforce_api_fieldmap_create($drupal, $salesforce, $automatic) {
  // Create the fieldmap array.
  $map = array(
    'index' => salesforce_api_fieldmap_next_index(),
    'drupal' => $drupal,
    'salesforce' => $salesforce,
    'automatic' => $automatic,
    'fields' => array(),
  );

  // Save the new fieldmap.
  salesforce_api_fieldmap_save($map);

  return $map['index'];
}

// Displays the confirm form for deleting a fieldmap.
function salesforce_api_fieldmap_delete_form(&$form_state, $index) {
  // Load the fieldmap from the database.
  $map = salesforce_api_fieldmap_load($index);

  // Return to the admin page if the fieldmap did not exist.
  if (empty($map)) {
    drupal_set_message(t('That fieldmap does not exist.'), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  }

  $form = array();

  // Add the index to the form array.
  $form['fieldmap_index'] = array(
    '#type' => 'value',
    '#value' => $index,
  );

  // Build the description text for this fieldmap.
  if ($map['action'] == 'import') {
    $desc = t('Fieldmap @index maps Salesforce %salesforce objects to Drupal %drupal objects for import.', array('@index' => $map['index'], '%drupal' => $map['drupal'], '%salesforce' => $map['salesforce']));
  }
  else {
    $desc = t('Fieldmap @index maps Drupal %drupal objects to Salesforce %salesforce objects for export.', array('@index' => $map['index'], '%drupal' => $map['drupal'], '%salesforce' => $map['salesforce']));
  }

  return confirm_form($form, t('Are you sure you want to delete this fieldmap?'), SALESFORCE_PATH_FIELDMAPS, $desc, t('Delete'));
}

function salesforce_api_fieldmap_delete_form_submit($form, &$form_state) {
  // Delete the specified fieldmap.
  salesforce_api_fieldmap_delete($form_state['values']['fieldmap_index']);

  // Display a message and return to the admin screen.
  drupal_set_message(t('The fieldmap has been deleted.'));

  $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS;
}


// Displays the edit form for adding field associations to a fieldmap.
function salesforce_api_fieldmap_edit_form(&$form_state, $index) {
  // Load the fieldmap from the database.
  $map = salesforce_api_fieldmap_load($index);

  // Return to the admin page if the fieldmap did not exist.
  if (empty($map)) {
    drupal_set_message(t('That fieldmap does not exist.'), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  }

  // Include the CSS file for the form.
  drupal_add_css(drupal_get_path('module', 'salesforce_api') .'/salesforce_api.admin.css');

  $form = array();

  // Add the index to the form array.
  $form['fieldmap_index'] = array(
    '#type' => 'value',
    '#value' => $index,
  );

  // Add a description of the source fieldmap to the form array.
  $form['fieldmap_desc'] = array(
    '#value' => '<p>'. salesforce_api_fieldmap_description($map) .'</p>',
  );

  $source = salesforce_api_fieldmap_objects_load('drupal', $map['drupal']);
  $target = salesforce_api_fieldmap_objects_load('salesforce', $map['salesforce']);

  // Fail with an error message if either the source or target object
  // definitions were not found.
  if (empty($source) || empty($target)) {
    drupal_set_message(t('This fieldmap cannot be edited, because an object definition could not be found.'), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  }

  // Add the data to the form for the required fields table.
  $form['fields'] = array(
    '#theme' => 'salesforce_api_fieldmap_edit_form_table',
    'rows' => array(),
  );

  $form['fields']['header'] = array(
    array('#value' => t('Target: @label (@type)', array('@label' => $target['label'], '@type' => $map['action'] == 'import' ? t('Drupal') : t('Salesforce')))),
    array('#value' => t('Source: @label (@type)', array('@label' => $source['label'], '@type' => $map['action'] == 'import' ? t('Salesforce') : t('Drupal')))),
  );

  // Loop through each of the target fields.
  $rows = array('required' => array(), 'optional' => array());
  foreach ($target['fields'] as $key => $value) {
    // Skip fields that are automatically assigned upon create for now.
    if ($value['type'] == SALESFORCE_FIELD_SOURCE_ONLY || $value['label'] == 'Owner ID') {
      continue;
    }

    // Determine to which table this field should belong.
    if ($value['type'] == SALESFORCE_FIELD_REQUIRED) {
      $type = 'required';
      $required = ' <span class="form-required" title="'. t('This field is required.') .'">*</span>';
    }
    else {
      $type = 'optional';
      $required = '';
    }

    // Create a row for this field.
    $row = array(
      'target' => array('#value' => $value['label'] . $required),
    );

    // Add the select list for the associated target field.
    $row['source'][$key] = array(
      '#type' => 'select',
      '#title' => $value['label'],
      '#options' => salesforce_api_fieldmap_field_options($source),
      '#default_value' => $map['fields'][$key],
      '#required' => $type == 'required',
    );

    // Add the row to the correct rows array.
    $rows[$type][] = $row;
  }

  // Combine the rows arrays into one with required fields displayed first.
  $form['fields']['rows'] = array_merge($rows['required'], $rows['optional']);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => l(t('Cancel'), SALESFORCE_PATH_FIELDMAPS),
  );

  return $form;
}

function salesforce_api_fieldmap_edit_form_submit($form, &$form_state) {
  // Load the fieldmap from the database.
  $map = salesforce_api_fieldmap_load($form_state['values']['fieldmap_index']);

  // Reset the fields array on the fieldmap.
  $map['fields'] = array();

  // Get the object definition for the target object.
  if ($map['action'] == 'import') {
    $object = salesforce_api_fieldmap_objects_load('drupal', $map['drupal']);
  }
  else {
    $object = salesforce_api_fieldmap_objects_load('salesforce', $map['salesforce']);
  }

  // Loop through all the fields on the object.
  foreach (array_keys($object['fields']) as $field) {
    // If a field has been mapped to this field on the form...
    if (!empty($form_state['values'][$field])) {
      // Add the association to the fieldmap's fields array.
      $map['fields'][$field] = $form_state['values'][$field];
    }
  }

  // Save the updated fieldmap.
  salesforce_api_fieldmap_save($map);

  // Display a message and return to the admin page.
  drupal_set_message(t('The changes have been saved.'));

  $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS;
}

// Themes the field associations on a fieldmap edit form into a table.
function theme_salesforce_api_fieldmap_edit_form_table($form) {
  // Build the header array.
  $header = array();

  foreach (element_children($form['header']) as $element) {
    $header[] = drupal_render($form['header'][$element]);
  }

  // Build the rows array.
  $rows = array();

  foreach (element_children($form['rows']) as $element) {
    $rows[] = array(
      drupal_render($form['rows'][$element]['target']),
      array('data' => drupal_render($form['rows'][$element]['source']), 'class' => 'source-cell'),
    );
  }

  // Add a message if no rows were found.
  if (empty($rows)) {
    $rows[] = array(
      array('data' => t('There are no fields of this type to set.'), 'colspan' => 2),
    );
  }

  // Build the attributes array.
  $attributes = array();

  // Build the caption.
  $caption = NULL;

  if (isset($form['caption'] )) {
    $caption = drupal_render($form['caption']);
  }

  return theme('table', $header, $rows, $attributes, $caption);
}


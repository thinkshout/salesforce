<?php
// $Id$

/**
 * @file
 * Contains the admin page callbacks for the Salesforce module, including forms
 *   for general settings and fieldmap administration.
 */

/**
 * The settings form at admin/settings/salesforce.
 */
function salesforce_api_settings_form() {
  $form = array();

  // Use the username field to collapse the API settings fieldset.
  $username = variable_get('salesforce_api_username', '');

  $form['api'] = array(
    '#type' => 'fieldset',
    '#title' => t('Salesforce API settings'),
    '#description' => t('Use your Salesforce.com login information for these username and password fields.'),
    '#collapsible' => !empty($username),
    '#collapsed' => !empty($username),
    '#weight' => -10,
  );
  $form['api']['salesforce_api_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#description' => t('Should be in the form of an e-mail address.'),
    '#default_value' => variable_get('salesforce_api_username', ''),
    '#required' => TRUE,
  );
  $form['api']['salesforce_api_password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#description' => t('Enter the password used when logging into Salesforce.'),
    '#default_value' => variable_get('salesforce_api_password', ''),
  );
  $form['api']['salesforce_api_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Security token'),
    '#description' => t('Set your security token by logging into Salesforce and navigating to Setup > My Personal Information > Reset My Security Token.'),
    '#required' => TRUE,
    '#default_value' => variable_get('salesforce_api_token', ''),
  );

  $form['log'] = array(
    '#type' => 'fieldset',
    '#title' => t('Log settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => -9,
  );
  $form['log']['salesforce_api_activity_log'] = array(
    '#type' => 'radios',
    '#title' => t('Activity log level'),
    '#options' => array(
      SALESFORCE_LOG_NONE => t('Do not log any Salesforce activities.'),
      SALESFORCE_LOG_SOME => t('Log important Salesforce activities.'),
      SALESFORCE_LOG_ALL => t('Log all Salesforce activitiies.'),
    ),
    '#default_value' => variable_get('salesforce_api_activity_log', SALESFORCE_LOG_SOME),
  );
  $form['log']['salesforce_api_error_log'] = array(
    '#type' => 'radios',
    '#title' => t('Error log level'),
    '#options' => array(
      SALESFORCE_LOG_NONE => t('Do not log any Salesforce errors.'),
      SALESFORCE_LOG_SOME => t('Log important Salesforce errors.'),
      SALESFORCE_LOG_ALL => t('Log all Salesforce errors.'),
    ),
    '#default_value' => variable_get('salesforce_api_error_log', SALESFORCE_LOG_ALL),
  );
  $form['objects'] = array(
    '#type' => 'fieldset',
    '#title' => t('Object settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => -9,
  );
  $form['objects']['clear_cache'] = array(
    '#type'  => 'item',
    '#value' => t('Caching data improves performance, however your Drupal site will be unaware of any alterations made to your Salesforce installation unless the cached data is refreshed. Select the lifetime the object data after which the cache will be automatically refreshed.  To refresh all cached object data on your site, click the button below.'),
  );
  $period = drupal_map_assoc(array(32400, 43200, 86400, 172800, 259200, 604800, 1209600, 2419200, 4838400, 9676800), 'format_interval');
  $period[CACHE_PERMANENT] = t('<none>');
  $form['objects']['salesforce_api_object_expire'] = array(
    '#type' => 'select',
    '#title' => t('Cache lifetime'),
    '#options' => $period,
    '#default_value' => variable_get('salesforce_api_object_expire', CACHE_PERMANENT),
  );
  $form['objects']['clear_cache_clear'] = array(
    '#type' => 'submit',
    '#value' => t('Clear cached object data'),
    '#submit' => array('salesforce_api_cache_build'),
  );
  // Validate handler makes sure that the salesforce_api_password doesn't get set to null on accident
  $form['#validate'][] = 'salesforce_api_settings_form_validate';
  $form['#submit'][] = 'salesforce_api_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Settings form validate handler to verify new salesforce credentials before saving them.
 */
function salesforce_api_settings_form_validate(&$form, &$form_state) {
  $values = $form_state['values'];
  if (!salesforce_api_connect($values['salesforce_api_username'], $values['salesforce_api_password'], $values['salesforce_api_token'], TRUE)) {
    // If not, prevent the user from overwriting the current configuration.
    form_set_error('salesforce_api_username', t('Unable to connect to Salesforce. Please check your credentials.'));
  }
  else {
    drupal_set_message(t('Salesforce connection established.'));
  }
}

/**
 * Settings form submit handler so that password doesn't get deleted.
 */
function salesforce_api_settings_form_submit(&$form, &$form_state) {
  // If the user hit "Save Configuration" and the required field
  // salesforce_api_password is blank, try to get it from variables
  if ($form_state['values']['submit'] == $form_state['values']['op'] and empty($form_state['values']['salesforce_api_password'])) {
    $pass = variable_get('salesforce_api_password', FALSE);
    if (isset($pass)) {
      $form_state['values']['salesforce_api_password'] = $pass;
    }
  }
}

/**
 * Displays an admin table for fieldmaps.
 */
function salesforce_api_fieldmap_admin() {
  // Define the header for the admin table.
  $header = array(t('Index'), t('Drupal object'), t('Salesforce object'), t('Automatic'), array('data' => t('Operations'), 'colspan' => 3));
  $rows = array();

  // Loop through all the indexed field maps.
  $result = db_query("SELECT fieldmap, drupal, salesforce, automatic FROM {salesforce_field_map}");
  while ($map = db_fetch_array($result)) {
    // Add the row to the table with the basic operations.
    $rows[] = array(
      $map['fieldmap'],
      salesforce_api_fieldmap_object_label('drupal', $map['drupal']),
      salesforce_api_fieldmap_object_label('salesforce', $map['salesforce']),
      $map['automatic'] ? t('Yes') : t('No'),
      l(t('edit'), SALESFORCE_PATH_FIELDMAPS .'/'. $map['fieldmap'] .'/edit'),
      l(t('delete'), SALESFORCE_PATH_FIELDMAPS .'/'. $map['fieldmap'] .'/delete'),
    );
  }

  // Add a message if no objects have been mapped.
  if (count($rows) == 0) {
    $rows[] = array(
      array('data' => t('You have not yet defined any fieldmaps.'), 'colspan' => 7),
    );
  }

  return theme('table', $header, $rows);
}

/**
 * Displays the form to add a fieldmap.
 */
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

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Map object fields'),
    '#suffix' => l(t('Cancel'), SALESFORCE_PATH_FIELDMAPS),
  );

  return $form;
}

/**
 * FAPI submit handler for a new fieldmap
 */
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
    'drupal' => $drupal,
    'salesforce' => $salesforce,
    'automatic' => $automatic,
    'fields' => array(),
  );

  // Save the new fieldmap.
  salesforce_api_fieldmap_save($map);

  return $map['fieldmap'];
}

/**
 * Displays the confirm form for deleting a fieldmap.
 */
function salesforce_api_fieldmap_delete_form(&$form_state, $fieldmap) {
  // Load the fieldmap from the database.
  $map = salesforce_api_fieldmap_load($fieldmap);

  // Return to the admin page if the fieldmap did not exist.
  if (empty($map)) {
    drupal_set_message(t('That fieldmap does not exist.'), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  }

  $form = array();

  // Add the fieldmap to the form array.
  $form['fieldmap_index'] = array(
    '#type' => 'value',
    '#value' => $fieldmap,
  );

  // Build the description text for this fieldmap.
  if ($map['action'] == 'import') {
    $desc = t('Fieldmap @index maps Salesforce %salesforce objects to Drupal %drupal objects for import.', array('@index' => $map['fieldmap'], '%drupal' => $map['drupal'], '%salesforce' => $map['salesforce']));
  }
  else {
    $desc = t('Fieldmap @index maps Drupal %drupal objects to Salesforce %salesforce objects for export.', array('@index' => $map['fieldmap'], '%drupal' => $map['drupal'], '%salesforce' => $map['salesforce']));
  }

  return confirm_form($form, t('Are you sure you want to delete this fieldmap?'), SALESFORCE_PATH_FIELDMAPS, $desc, t('Delete'));
}

/**
 * FAPI submit handler for deleting a fieldmap
 */
function salesforce_api_fieldmap_delete_form_submit($form, &$form_state) {
  // Delete the specified fieldmap.
  salesforce_api_fieldmap_delete($form_state['values']['fieldmap_index']);

  // Display a message and return to the admin screen.
  drupal_set_message(t('The fieldmap has been deleted.'));

  $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS;
}

/**
 * Displays the edit form for adding field associations to a fieldmap.
 */
function salesforce_api_fieldmap_edit_form(&$form_state, $fieldmap) {
  // Load the fieldmap from the database.
  $map = salesforce_api_fieldmap_load($fieldmap);

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
    '#value' => $fieldmap,
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

  $form['drupal_sfapi_automatic'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically Populate Salesforce?'),
    '#return_value' => 1,
    '#default_value' => $map['automatic'],
    '#description' => t('Automatically create and link new salesforce objects when Drupal objects are created?'),
  );

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

/**
 * FAPI submit handler for fieldmap editor
 */
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

  // set the automatic flag on the map
  $map['automatic'] = $form_state['values']['drupal_sfapi_automatic'];

  // Save the updated fieldmap.
  salesforce_api_fieldmap_save($map);

  // Display a message and return to the admin page.
  drupal_set_message(t('The changes have been saved.'));

  $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS;
}

/**
 * Themes the field associations on a fieldmap edit form into a table.
 */
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

 /**
 * Ask salesforce for a list of objects and display a checklist for the user.
 * Based on user selection, set up or tear down cached/synched Salesforce data.
 * @TODO make this more user friendly. At the moment it's possible for an admin user to blow away
 * their entire local SalesForce cache with a few clicks. This is not necessarily desirable.
 *
 * @param string $form_state
 * @return void
 * @author aaron
 */
function salesforce_api_admin_object(&$form_state) {
  $response = salesforce_api_describeGlobal();
  if (empty($response->types)) {
    drupal_set_message(t('There was an error retrieving the list of SalesForce objects. Please verify that your SalesForce instance is properly configured.'), 'error');
    return;
  }
  $defaults = array_keys(salesforce_api_fieldmap_objects());
  
  $options = array();
  foreach($response->types as $obj) {
    $options[$obj->name] = $obj->name . ' (' . $obj->label .')';
  }

  // Disable any SF Object types currently in use by fieldmap(s).
  $result = db_query('SELECT DISTINCT salesforce FROM {salesforce_field_map}');
  while ($type = db_result($result)) {
    $disabled[$type] = $type;
  }

  $fields = array(
    'objects' => array(
      '#type' => 'checkboxes',
      '#title' => 'Object Name (Object Label)',
      '#description' => 'Check the SalesForce objects you would like to synchronize locally.',
      '#options' => $options,
      '#default_value' => $defaults,
    ),
    'disabled_types' => array('#type' => 'value', '#value' => $disabled),
    '#theme' => 'salesforce_api_object_options',
    'submit' => array(
      '#type' => 'submit',
      '#value' => 'Save'
      ),
    );
  return $fields;
}

/**
 * FAPI submit handler
 * Gather enabled SF Objects and rebuild the cache.
 */
function salesforce_api_admin_object_submit($form, &$form_state) {
  $values = $form_state['values']['objects'];
  // Start off with all the SF Object types already in use.
  $real_types = $form['disabled_types']['#value'];
  foreach ($values as $i => $t) {
    if (empty($t)) {
      continue;
    }
    $real_types[$i] = $t;
  }
  if (empty($real_types)) {
    $real_types = array();
  }
  $sf_objects = variable_set('salesforce_api_enabled_objects', array_values($real_types));
  $objects = salesforce_api_cache_build();
  return;
}

/**
 * Placeholder for per-object configuration settings. Any ideas?
 */
function salesforce_api_admin_object_settings($form_state, $type) {
  return array('settings' => array(
    '#type' => 'markup',
    '#value' => 'Placeholder for per-object configuration settings.',
    'description' => array(
      '#type' => 'fieldset',
      '#title' => 'Object Schema',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => '<pre>' . print_r(salesforce_api_describeSObject($type), 1) . '</pre>',
    )));
}

/**
 * Theming function for salesforce_api_admin_setup
 * For locally-cached SF Objects, add a "configure" or "re-map" link next to the checkbox
 */
function theme_salesforce_api_object_options($element = NULL) {
  if (empty($element['objects']['#options'])) {
    return drupal_render($element);
  }
  $objects = $element['objects'];
  $options = $objects['#options'];
  // Disable checkboxes for SF Types which are currently in use.
  foreach ($options as $key => $value) {
    if (empty($objects[$key]['#value'])) {
      continue;
    }
    if (!empty($element['disabled_types']['#value'][$key])) {
      $element['objects'][$key]['#attributes']['disabled'] = 'disabled';
      $element['objects'][$key]['#description'] =
        t('This object is in use by one or more fieldmaps and cannot be disabled.');
    }
    $link = l('configure', SALESFORCE_PATH_OBJECT .'/'. $key);
    if ($_SESSION['objects_error'][$key]) {
      $element['objects'][$key]['#prefix'] = '<div class="error">';
      $element['objects'][$key]['#suffix'] = '</div>';
      unset($_SESSION['objects_error'][$key]);
      $link = l('re-map fields', SALESFORCE_PATH_OBJECT .'/'. $key);
    }
    $element['objects'][$key]['#title'] .= ' | '. $link;
  }
  unset($_SESSION['objects_error']);
  return drupal_render($element);
}

/**
 * Demonstrates some of the API functionality through the Salesforce class and
 * fieldmap functionality. 
 *
 * @param $demo
 *   The name of the demonstration to perform.
 * @return
 *   A string containing the page output.
 * @todo
 *  2/3 of the demo functionality is currently broken. see #692378
 * @see http://drupal.org/node/692378
 */
function salesforce_api_demo($demo = NULL, $arg = NULL) {
  // Attempt to connect to Salesforce.
  $sf = salesforce_api_connect();

  // Display an error message if the connection failed.
  if (!$sf) {
    return t('Could not connect to Salesforce. Please doublecheck your API credentials.');
  }

  // Display the server timestamp first.
  $response = $sf->client->getServerTimestamp();

  $output = '<p>'. t('<b>Salesforce server timestamp:</b> @timestamp', array('@timestamp' => $response->timestamp)) .'</p>';

  // Add a specific demo's output.
  switch ($demo) {
    case 'data-structure':
      if ($arg) {
        $response = salesforce_api_describeSObjects(array(check_plain($arg)));
        if (function_exists('dpm')) {
          dpm($response);
        }
        elseif (function_exists('krumo')) {
          $output .= krumo($response);
        }
        else {
          $output .= '<pre>'. print_r($response, 1) .'</pre>';
        }
      }
      else {
        $response = salesforce_api_describeGlobal();
        if (is_array($response->types)) {
          foreach ($response->types as $type) {
            $items[] = l($type, SALESFORCE_PATH_DEMO .'/data-structure/'. $type);
          }
          $output .= theme('item_list', $items, t('Global Data Structure'));
        }
      }

      break;
    case 'user-export':
      // TODO: This is broken. See http://drupal.org/node/692378
      break;
      
      global $user;

      // Fully load the user account so we get profile data.
      $account = user_load(array('uid' => $user->uid));

      // Create an object for export based on fieldmap 1.
      $object = salesforce_api_fieldmap_export_create(1, $account);

      $output .= t('Your user account prepared for export as a Contact based on <a href="!url">fieldmap 1</a>:', array('!url' => url(SALESFORCE_PATH_FIELDMAPS .'/1/edit')))
                .'<pre>'. print_r($object, TRUE) .'</pre>';

      // Export the object to Salesforce as a Contact.
      $response = $sf->client->create(array($object), 'Contact');

      $output .= t('The response from Salesforce upon creation:')
                .'<pre>'. print_r($response, TRUE) .'</pre>';
      break;

    case 'campaign-export':
      // TODO: Also broken. See http://drupal.org/node/692378
      break;
      
      // Load the campaign node.
      $node = node_load(1);

      // Create an object for export based on fieldmap 10.
      $object = salesforce_api_fieldmap_export_create(10, $node);

      $output .= t('Your campaign prepared for export as a Campaign based on <a href="!url">fieldmap 10</a>:', array('!url' => url(SALESFORCE_PATH_FIELDMAPS .'/10/edit')))
                .'<pre>'. print_r($object, TRUE) .'</pre>';

      // Export the object to Salesforce as a Contact.
      $response = $sf->client->create(array($object), 'Campaign');

      $output .= t('The response from Salesforce upon creation:')
                .'<pre>'. print_r($response, TRUE) .'</pre>';
      break;
  }

  $items = array(
    l(t('Examine Data Structure'), SALESFORCE_PATH_DEMO .'/data-structure'),
// TODO: broken. See http://drupal.org/node/692378
//    l(t('Export your user account as a contact'), SALESFORCE_PATH_DEMO .'/user-export'),
//    l(t('Export node 1 as a campaign'), SALESFORCE_PATH_DEMO .'/campaign-export'),
  );

  $output .= '<p>'. t('<b>Choose from the following demonstrations:</b>') . theme('item_list', $items) .'</p>';

  return $output;
}

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
    '#title' => t('Minimum cache lifetime'),
    '#options' => $period,
    '#default_value' => variable_get('salesforce_api_object_expire', CACHE_PERMANENT),
  );
  $form['objects']['clear_cache_clear'] = array(
    '#type' => 'submit',
    '#value' => t('Clear cached object data'),
    '#submit' => array('salesforce_api_cache_build'),
  );

  // WSDL file should be outside of webroot. The admin should specify a path
  // outside the webroot, then upload a WSDL file to it.
  $wsdl_dir = variable_get('salesforce_api_dir_wsdl', FALSE);
  $form['wsdl'] = array(
    '#type' => 'fieldset',
    '#title' => 'WSDL directory',
    '#description' => 'Your organization\'s WSDL file can expose potentially sensitive information. It is highly recommended that your WSDL file be stored outside your webroot. Please enter either a path relative to your webroot (e.g. ../wsdl) or a fully qualified path (e.g. /home/username/wsdl) in which to store your WSDL.',
    'salesforce_api_dir_wsdl' => array(
      '#type' => 'textfield',
      '#title' => 'WSDL Directory',
      '#default_value' => $wsdl_dir,
      ),
    '#collapsible' => TRUE,
    '#collapsed' => $wsdl_dir,
    );

  // Validate handler makes sure that the salesforce_api_password doesn't get set to null on accident
  $form['#validate'][] = 'salesforce_api_settings_form_validate';
  $form['#submit'][] = 'salesforce_api_settings_form_submit';

  return system_settings_form($form);
}

/**
 * Settings form validate handler to verify new salesforce credentials before saving them.
 */
function salesforce_api_settings_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (!salesforce_api_connect($values['salesforce_api_username'], $values['salesforce_api_password'], $values['salesforce_api_token'], TRUE)) {
    // If not, prevent the user from overwriting the current configuration.
    form_set_error('salesforce_api_username', t('Unable to connect to Salesforce. Please check your credentials.'));
  }
  else {
    drupal_set_message(t('Salesforce connection established.'));
  }

  if (isset($form_state['values']['salesforce_api_dir_wsdl'])
  && !file_exists($form_state['values']['salesforce_api_dir_wsdl'])) {
    form_set_error('salesforce_api_dir_wsdl', 'The specified directory does not exist. Please make sure the directory exists, check your input, and try again.');
  }
}

/**
 * Settings form submit handler so that password doesn't get deleted.
 */
function salesforce_api_settings_form_submit($form, &$form_state) {
  // If the user hit "Save Configuration" and the required field
  // salesforce_api_password is blank, try to get it from variables
  if ($form_state['values']['submit'] == $form_state['values']['op'] and empty($form_state['values']['salesforce_api_password'])) {
    $pass = variable_get('salesforce_api_password', FALSE);
    if (isset($pass)) {
      $form_state['values']['salesforce_api_password'] = $pass;
    }
  }
  if (variable_get('salesforce_api_dir_wsdl', FALSE) !=
      $form_state['values']['salesforce_api_dir_wsdl']) {
    drupal_set_message('Please make sure the WSDL directory is writeable, and upload a valid Salesforce .xml or .wsdl file.');
    $form_state['redirect'] = array(
      SALESFORCE_PATH_UPDATE_WSDL,
      'destination=' . SALESFORCE_PATH_ADMIN);
    ini_set('soap.wsdl_cache_enabled',  '0');
  }
}

/**
 * Displays an admin table for fieldmaps.
 */
function salesforce_api_fieldmap_admin() {
  // Define the header for the admin table.
  $header = array(t('Drupal object'), t('Salesforce object'), t('Automatic'), t('Description'), array('data' => t('Operations'), 'colspan' => 4));
  $rows = $default_maps = $new_default_maps = array();
  $content = '';
  
  $maps = salesforce_api_salesforce_field_map_load_all();
  foreach($maps as $map) {
    $rows[] = _salesforce_api_field_admin_format_row($map);
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
 * Helper function for salesforce_api_field_admin
 * @see salesforce_api_field_admin_format_row
 */
function _salesforce_api_field_admin_format_row($map, $options = array()) {
  $options = array_merge(array('overridden' => FALSE), $options);
  // Add the row to the table with the basic operations.
  $base = SALESFORCE_PATH_FIELDMAPS .'/'. $map->name;

  switch($map->type) {
    case 'Normal': {
      $operations = array(
        l(t('edit'), $base . '/edit'),
        l(t('clone'), $base . '/clone'),
        l(t('delete'), $base . '/delete'));
      break;
    }
    case 'Default': {
      $operations = array(
        l(t('override'), $base . '/edit'),
        l(t('clone'), $base . '/clone'));
      break;
    }
    case 'Overridden': {
      $operations = array(
        l(t('edit'), $base . '/edit'),
        l(t('clone'), $base . '/clone'),
        l(t('revert'), $base . '/revert'));
      break;
    }
  }

  $operations = array_pad($operations, 3, '&nbsp;');
  
  if (module_exists('ctools')) {
    $operations[] = l(t('export'), $base . '/export');
  }

  $auto = array();
  switch ($map->automatic) {
    case SALESFORCE_AUTO_SYNC_CREATE: {
      $auto[] = t('Create');
      break;
    }
    case SALESFORCE_AUTO_SYNC_UPDATE: {
      $auto[] = t('Update');
      break;
    }    
    case SALESFORCE_AUTO_SYNC_DELETE: {
      $auto[] = t('Delete');
      break;
    }
  }
  $auto = implode(', ', $auto);
  $row = array(
    salesforce_api_fieldmap_object_label('drupal', $map->drupal),
    salesforce_api_fieldmap_object_label('salesforce', $map->salesforce),
    $auto,
    array('data' => check_plain($map->description), 
          'class' => 'description'));
  $row = array_merge($row, $operations);
  return $row;
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
  asort($options);
  $form['drupal'] = array(
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
  asort($options);
  $form['salesforce'] = array(
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
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => 'Title or short description',
    '#description' => t('Enter a brief description of this fielmap to distinguish it from potentially similar fieldmaps'),
  );

  return $form;
}

/**
 * FAPI submit handler for a new fieldmap
 */
function salesforce_api_fieldmap_add_form_submit($form, &$form_state) {
  // Create the new fieldmap.
  $map = salesforce_api_fieldmap_create($form_state['values']);
  $form_state['values']['fieldmap'] = $map;
  // Redirect to its edit form.
  $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS .'/'. $map->name .'/edit';
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
function salesforce_api_fieldmap_create($values) {
  // Create the fieldmap array.
  $map = (object)($values + array('fields' => array()));

  // Save the new fieldmap.
  salesforce_api_fieldmap_save($map);

  return $map;
}

/**
 * Displays the form for cloning a fieldmap
 *
 */
function salesforce_api_fieldmap_clone_form(&$form_state, $fieldmap) {
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
  $desc = salesforce_api_fieldmap_description($map);

  return confirm_form($form, t('Are you sure you want to clone this fieldmap?'), SALESFORCE_PATH_FIELDMAPS, $desc, t('Clone'));

}

function salesforce_api_fieldmap_clone_form_submit($form, &$form_state) {
  // Clone the specified fieldmap.
  $map = salesforce_api_fieldmap_clone($form_state['values']['fieldmap_index']);

  if ($map) {
    // Display a message and return to the admin screen.
    drupal_set_message(t('Fieldmap cloned successfully.'));
    $form_state['redirect'] = 
      SALESFORCE_PATH_FIELDMAPS . '/' . $map->name . '/edit';
  }
  else {
    drupal_set_message(t('Failed to clone fieldmap.'), 'error');
  }
}

/**
 * Displays the confirm form for deleting a fieldmap.
 */
function salesforce_api_fieldmap_delete_form(&$form_state, $fieldmap) {
  $args = arg();
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
  $desc = salesforce_api_fieldmap_description($map);
  
  $action = end($args);
  $text = t('Are you sure you want to @action this fieldmap?', 
    array('@action' => $action));
  $button = t('@action', array('@action' => ucfirst($action)));
  return confirm_form($form, $text, SALESFORCE_PATH_FIELDMAPS, $desc, $button);

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
  
  // Include the CSS and JS for the form.
  $path = drupal_get_path("module", "salesforce_api");
  drupal_add_css($path .'/misc/salesforce_api.admin.css');
  drupal_add_js($path ."/misc/salesforce_api.admin.js"); 
  
  // Load the fieldmap from the database.
  $map = salesforce_api_fieldmap_load($fieldmap, TRUE);

  // Return to the admin page if the fieldmap did not exist.
  if (empty($map)) {
    drupal_set_message(t('That fieldmap does not exist.'), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  }

  // Include the CSS file for the form.
  $path = drupal_get_path('module', 'salesforce_api');
  drupal_add_css($path . '/misc/salesforce_api.admin.css');
  drupal_add_js($path . '/misc/salesforce_api.admin.js'); 

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

  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => 'Title or short description',
    '#description' => t('Enter a brief description of this fielmap to distinguish it from potentially similar fieldmaps'),
    '#default_value' => $map->description,
  );

  // Fail with an error message if either the source or target object
  // definitions were not found.
  if (!salesforce_api_fieldmap_source_enabled($map)) {
    drupal_set_message(t('This fieldmap cannot be edited because the module which supports the Drupal entity "%entity" cannot be found. Please make sure you have required any modules with which this fieldmap was built.', array('%entity' => $map->drupal)), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  } 
  if (!salesforce_api_fieldmap_target_enabled($map)) {
    drupal_set_message(t('This fieldmap cannot be edited because salesforce_api cannot find a definition for the Salesforce object "%sfobj". Please verify your Salesforce connection and settings.', array('%sfobj' => $map->salesforce)), 'error');
    drupal_goto(SALESFORCE_PATH_FIELDMAPS);
  }

  $source = salesforce_api_fieldmap_objects_load('drupal', $map->drupal);
  $target = salesforce_api_fieldmap_objects_load('salesforce', $map->salesforce);

  $automatic = array(
    SALESFORCE_AUTO_SYNC_CREATE =>
      $map->automatic & SALESFORCE_AUTO_SYNC_CREATE,
    SALESFORCE_AUTO_SYNC_UPDATE =>
      $map->automatic & SALESFORCE_AUTO_SYNC_UPDATE,
    SALESFORCE_AUTO_SYNC_DELETE =>
      $map->automatic & SALESFORCE_AUTO_SYNC_DELETE,
    );
  $form['drupal_sfapi_automatic'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Synchronize automatically with Salesforce on (check all that apply)'),
    '#options' => array(
      SALESFORCE_AUTO_SYNC_CREATE => t('Create'), 
      SALESFORCE_AUTO_SYNC_UPDATE => t('Update'),
      SALESFORCE_AUTO_SYNC_DELETE => t('Delete'),
      // @see theme_salesforce_api_drupal_sfapi_automatic for option descriptions
    ),
    '#return_value' => 1,
    '#default_value' => $automatic,
    '#theme' => 'salesforce_api_drupal_sfapi_automatic',
    '#description' => t('Please indicate how Salesforce records should be handled when Drupal records are created or updated.')
  );

  // Get a list of Salesforce-side fields which haven't been set yet.
  asort($target['fields']);
  $not_set = array('');
  foreach($target['fields'] as $key => $field) {
    if(!isset($map->fields[$key])) {
      $not_set[$key] = $field['label'];
    }
  }

  // Provide a control for setting a new field
  $form['add_field'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add a field'),
    'new_field' => array(
      '#type' => 'select',
      '#options' => $not_set,
    ),
    'add_button' => array(
      '#type' => 'submit',
      '#value' => t('Add'),
    ),
  );

  // Add the data to the form for the required fields table.
  $form['fields'] = array('#theme' => 'salesforce_api_fieldmap_edit_form_table');

  $form['fields']['header'] = array(
    array('#value' => t('Target: @label', array('@label' => $target['label']))),
    array('#value' => t('Source: @label', array('@label' => $source['label']))),
  );

  // Adding fixed value option
  $options = salesforce_api_fieldmap_field_options($source);
  $options['Other']['fixed'] = t('Fixed value');
  if( user_access('use php for salesforce fixed values') ) {
    $options['Other']['php'] = t('Evaluate PHP');
  }

  // Loop through each of the target fields.
  $rows = array('required' => array(), 'optional' => array());
  $form['fake_required'] = array('#type' => 'value', '#value' => array());

  foreach ($target['fields'] as $key => $value) {
    // Determine to which table this field should belong.
    if (!($value['type'] & (SALESFORCE_FIELD_NILLABLE |   SALESFORCE_FIELD_DEFAULTEDONCREATE))) {
      // If the field is not nillable and not defaulted on create, then it must be required.
      $type = 'required';
      $required = ' <span class="form-required" title="'. t('This field is required.') .'">*</span>';
    }
    else {
      $type = 'optional';
      $required = '';
    }

    if ($value['type'] & SALESFORCE_FIELD_CREATEABLE
    && !($value['type'] & SALESFORCE_FIELD_UPDATEABLE)) {
      $type = 'optional';
      $required = ' <span class="form-required" title="' .
        t('This field will only be set for new records.') . '">' . t('Create-only') . '</span>';
    }
    elseif (!($value['type'] & SALESFORCE_FIELD_CREATEABLE)
    && $value['type'] & SALESFORCE_FIELD_UPDATEABLE) {
      $type = 'optional';
      $required = ' <span class="form-required" title="' .
        t('This field can only be set for existing records.') . '">' . t('Update-only') . '</span>';
    }
    elseif (!($value['type'] & (SALESFORCE_FIELD_CREATEABLE | SALESFORCE_FIELD_UPDATEABLE))) {
      $type = 'optional';
      $required = ' <span class="form-required" title="' .
        t('This field will be available for imports only.') . '">' . t('Read-only') . '</span>';
    }

    if ($type == 'optional' && !isset($map->fields[$key])) {
      continue;
    }

    // Create a row for this field.
    $row = array(
      'target' => array('#value' => $value['label'] . $required),
    );

    if (is_array($map->fields[$key])) {
      $default_key = $map->fields[$key]['type'];
      $default_value = $map->fields[$key]['value'];
    } else {
      $default_key = $map->fields[$key];
      $default_value = null;
    }

    // Add the select list for the associated target field.
    $row['source'][$key] = array(
      '#type' => 'select',
      '#title' => $value['label'],
      '#options' => $options,
      '#default_value' => $default_key,
      '#attributes' => array('class' => 'sf_fieldmap_options', 'id' => 'sf-fieldmap-option-' . $key),
    );
    $row['source'][$key ."_extra"] = array(
      '#type' => 'textfield',
      '#title' => t('Value'),
      '#default_value' => $default_value,
      '#size' => 20,
      '#maxlength' => 512,
      '#prefix' => '<div id="'. $key .'-extra-hidden" class="fieldmap-extra-text">',
      '#suffix' => '</div>',
      '#description' => 'Omit &lt;?php ?&gt; tags. Return the value to set. Standard caveats apply.',
    );
    
    if ($type != 'required') {
      $row['remove'][$key] = array(
        '#type' => 'markup',
        '#value' => l('Remove', SALESFORCE_PATH_FIELDMAPS . '/' . $map->name . '/remove/' . $key),
      );
    }
    else {
      $form['fake_required']['#value'][] = $key;
    }

    // Add the row to the correct rows array.
    $rows[$type][$key] = $row;
  }

  // Combine the rows arrays into one with required fields displayed first.
  $form['fields']['rows'] = array_merge($rows['required'], $rows['optional']);
  $form['fields']['rows']['#tree'] = TRUE;
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
    '#suffix' => l(t('Cancel'), SALESFORCE_PATH_FIELDMAPS),
  );

  return $form;
}

/**
 * FAPI validate handler for fieldmap editor
 *
 */
function salesforce_api_fieldmap_edit_form_validate($form, &$form_state) {
  // Include the CSS file for the form on reload as long as Drupal won't do it for us.
  $path = drupal_get_path('module', 'salesforce_api');
  drupal_add_css($path . '/misc/salesforce_api.admin.css');
  drupal_add_js($path . '/misc/salesforce_api.admin.js'); 

  if ($form_state['values']['add_button'] == $form_state['values']['op']
  && empty($form_state['values']['new_field'])) {
    form_set_error('new_field', t('Please select a target field.'));
  }
  elseif ($form_state['values']['submit'] == $form_state['values']['op']
  && !empty($form_state['values']['fake_required'])
  && empty($form_state['values']['new_field'])) {
    $rows = $form_state['values']['rows'];
    foreach ($form_state['values']['fake_required'] as $name) {
      if (empty($form_state['values']['rows'][$name]['source'][$name])) {
        form_error($form['fields']['rows'][$name]['source'][$name], '"' . $form['fields']['rows'][$name]['source'][$name]['#title'] . '" field is required.');
      }
    }
  }
}

/**
 * FAPI submit handler for fieldmap editor
 */
function salesforce_api_fieldmap_edit_form_submit($form, &$form_state) {
  // Load the fieldmap from the database.
  $map = salesforce_api_fieldmap_load($form_state['values']['fieldmap_index']);

  $map->description = $form_state['values']['description'];
  $map->fields = array();

  // Get the object definition for the target object.
  $object = salesforce_api_fieldmap_objects_load('salesforce', $map->salesforce);
  $rows = $form_state['values']['rows'];

  // Loop through all the select inputs. 
  foreach ($rows as $row) {
    $source = key($row['source']);
    $target = current($row['source']);
    if (!empty($row['source'][$source . '_extra'])) {
      $map->fields[$source] = array(
        'type' => $target, 
        'value' => $row['source'][$source . '_extra'],
      );
    }
    else {
      // Add the association to the fieldmap's fields array.
      $map->fields[$source] = $target;
    }
  }

  $map->automatic = 0;
  // set the automatic flag on the map
  foreach ($form_state['values']['drupal_sfapi_automatic'] as $auto) {
    $map->automatic |= $auto;
  }
  
  if(!empty($form_state['values']['new_field'])) {
    $field = $form_state['values']['new_field'];
    $map->fields[$field] = '';
  }

  // Save the updated fieldmap.
  salesforce_api_fieldmap_save($map);

  // In most cases, we will want to remain on this page and rebuild the form.
  $form_state['rebuild'] = TRUE;
  switch ($form_state['values']['op']) {
    case $form_state['values']['add_button']: {
      drupal_set_message(t('Field added.'));
      break;
    }
    case $form_state['values']['submit']: {
      if(!empty($form_state['values']['new_field'])) {
        drupal_set_message(t('Field added.'));
      }
      else {
        drupal_set_message(t('The changes have been saved.'));
        $form_state['redirect'] = SALESFORCE_PATH_FIELDMAPS;
        $form_state['rebuild'] = FALSE;
      }
      break;
    }
  }
}

/**
 * Given a fieldmap name and a field, remove the field from the fieldmap.
 * Redirect to the fieldmap edit page unless specified.
 * @param string or object $fieldmap - the name of the fieldmap, or the fieldmap itself
 * @param string $field - the name of the field to be removed
 * @param bool $redirect (optional) - default true. Whether to redirect to the fieldmap edit form
 */
function salesforce_api_fieldmap_remove_field($fieldmap, $field, $redirect = TRUE) {
  if (is_string($fieldmap)) {
    $fieldmap = salesforce_api_fieldmap_load($fieldmap);
  }
  $object = salesforce_api_fieldmap_objects_load('salesforce', $fieldmap->salesforce);
  $success = TRUE;
  if (empty($object->fields[$field])) {
    $success = FALSE;
  }
  elseif (!($object->fields[$field]['type'] & (SALESFORCE_FIELD_NILLABLE |   SALESFORCE_FIELD_DEFAULTEDONCREATE))) {
    // Don't allow removing required fields.
    $success = FALSE;
  }
  else {
    unset($fieldmap->fields[$field]);
    salesforce_api_fieldmap_save($fieldmap);
  }
  if ($redirect) {
    drupal_goto(SALESFORCE_PATH_FIELDMAPS . '/' . $fieldmap->name . '/edit');
    exit;
  }
  return $success;
}

/**
 * Theme callback for synch option radios
 */
function theme_salesforce_api_drupal_sfapi_automatic($element) {
  $element[SALESFORCE_AUTO_SYNC_CREATE]['#description'] = 
    t('Create and link Salesforce records automatically when Drupal records are created.');
  $element[SALESFORCE_AUTO_SYNC_UPDATE]['#description'] = 
    t('Update Salesforce records if a link already exists.');
  $element[SALESFORCE_AUTO_SYNC_DELETE]['#description'] = 
    t('Delete linked Salesforce records when a Drupal record is deleted.');
  return drupal_render($element);
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
      array('data' => drupal_render($form['rows'][$element]['target']), 'class' => 'target-cell'),
      array('data' => drupal_render($form['rows'][$element]['source']), 'class' => 'source-cell'),
      drupal_render($form['rows'][$element]['remove']),
    );
  }

  // Add a message if no rows were found.
  if (empty($rows)) {
    $rows[] = array(
      array('data' => t('No fields set.'), 'colspan' => 2),
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
 * Show form for admin to upload a new WSDL file
 */
function salesforce_api_update_wsdl_form() {
  $form = array();

  // Need to set the form type so file upload will work
  $form['#attributes']['enctype'] = 'multipart/form-data';

  $form['salesforce_api_wsdl_file'] = array(
    '#type' => 'file',
    '#title' => t('WSDL File'),
    '#description' => t('Upload the new WSDL definition file. The name of the file is irrelevent, but it must end with an XML or WSDL extension.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Upload WSDL'),
  );

  return $form;
}


/**
 * Validate the WSDL file upload - check upload error code and file extension
 * and do the actual upload processing so that we can invalidate the form on any
 * errors. This function SHOULD use file_save_upload(), but unfortunately, that
 * function calls file_create_path() which calls file_check_location(), which
 * fails on any destination dir that isn't in the default file upload dir. And
 * since the enterprise.wsdl.xml file needs to go into a sub-dir in the module
 * directory, that won't do. Instead of uploading to the default dir, then 
 * moving, just upload it straight to the destination.
 */
function salesforce_api_update_wsdl_form_validate($form, &$form_state) {
  $source = 'salesforce_api_wsdl_file';
  // Find the dir the file should go into
  if (!$dir = variable_get('salesforce_api_dir_wsdl', FALSE)) {
    $dir = drupal_get_path('module', 'salesforce_api') . '/wsdl';
  }
  $dir = rtrim($dir, '/');

  // Upload file path
  $file = $dir . '/enterprise.wsdl.xml';

  // Make sure the directory is writeable
  if( !file_check_directory( $dir ) ) {
    form_set_error($source, t('Server directory %directory is not writeable. Please check directory permissions or contact a site admin to correct this.', array('%directory' => $dir)));
    return;
  }

  if (!isset($_FILES['files']) && $_FILES['files']['name'][$source]
  || !is_uploaded_file($_FILES['files']['tmp_name'][$source])) {
    form_set_error($source, t('Error uploading file. Please try again.'));
    return;
  }
  // Check for file upload errors and return FALSE if a
  // lower level system error occurred.
  switch ($_FILES['files']['error'][$source]) {
    // @see http://php.net/manual/en/features.file-upload.errors.php
    case UPLOAD_ERR_OK:
      break;

    case UPLOAD_ERR_INI_SIZE:
    case UPLOAD_ERR_FORM_SIZE:
      drupal_set_message(t('The file %file could not be saved, because it exceeds %maxsize, the maximum allowed size for uploads.', array('%file' => $source, '%maxsize' => format_size(file_upload_max_size()))), 'error');
      return 0;

    case UPLOAD_ERR_PARTIAL:
    case UPLOAD_ERR_NO_FILE:
      drupal_set_message(t('The file %file could not be saved, because the upload did not complete.', array('%file' => $source)), 'error');
      return 0;

      // Unknown error
    default:
      drupal_set_message(t('The file %file could not be saved. An unknown error has occurred.', array('%file' => $source)), 'error');
      return 0;
  }

  // Check the file extension
  $pathParts = pathinfo($_FILES['files']['name'][$source]);
  if(!in_array(strtolower($pathParts['extension']), array('xml', 'wsdl'))) {
    form_set_error($source, t('File upload error: invalid file extension. Please upload a file with an XML or WSDL  extension.'));
  }

  // Try to move the uploaded file into the right place
  if (!move_uploaded_file($_FILES['files']['tmp_name'][$source], $file)) {
    form_set_error($source, t('File upload error. Could not move uploaded file.'));
    watchdog('file', 'Upload error. Could not move uploaded file to destination %destination.', array('%destination' => $dir));
  }
}

/**
 * If we get this far, we've successfully uploaded the new WSDL.
 * Clean up after ourselves and display the appropriate confirmation messages.
 */
function salesforce_api_update_wsdl_form_submit($form, &$form_state) {
  drupal_set_message(t('The WSDL file has been successfully uploaded. You should remove write permissions from the wsdl directory.'));
  
  // Disable PHP SOAP WSDL cache for this request. This doesn't disable the
  // cache permanently, but should force PHP to reload it.
  ini_set('soap.wsdl_cache_enabled',  '0');

  // Clear the cache
  drupal_flush_all_caches();
  
  // Clear WSDL files from tmp directory
  $tmpDir = file_directory_temp();
  // Make sure the tmp dir exists and isn't the root, just to be safe
  if(is_dir($tmpDir) && $tmpDir != '/') {
    $cmd = 'rm -f ' . $tmpDir . '/*.wsdl.*';
    $execOutput = exec($cmd);
  }
  
  drupal_set_message('Drupal cache emptied and WSDL files removed from temp directory.');

}

 /**
 * Ask salesforce for a list of objects and display a checklist for the user.
 * Based on user selection, set up or tear down cached/synched Salesforce data.
 * @TODO make this more user friendly. At the moment it's possible for an admin user to blow away
 * their entire local Salesforce cache with a few clicks. This is not necessarily desirable.
 *
 * @param string $form_state
 * @return void
 * @author aaron
 */
function salesforce_api_admin_object(&$form_state) {
  $response = salesforce_api_describeGlobal();
  if (empty($response->types)) {
    drupal_set_message(t('There was an error retrieving the list of Salesforce objects. Please verify that your Salesforce instance is properly configured.'), 'error');
    return;
  }
  $defaults = array_keys(salesforce_api_fieldmap_objects());

  $options = array();
  foreach($response->types as $obj) {
    $options[$obj->name] = $obj->name . ' (' . $obj->label .')';
  }

  // Disable any SF Object types currently in use by fieldmap(s).
  $result = db_query('SELECT DISTINCT salesforce FROM {salesforce_field_map}');
  $fieldmaps = salesforce_api_salesforce_field_map_load_all();
  foreach ($fieldmaps as $map) {
    $disabled[$map->salesforce] = $map->salesforce;
  }

  $fields = array(
    'objects' => array(
      '#type' => 'checkboxes',
      '#title' => 'Object Name (Object Label)',
      '#description' => 'Check the Salesforce objects you would like to synchronize locally.',
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
  $sf_objects = variable_set('salesforce_api_enabled_objects', array_filter(array_values($real_types)));
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
  global $user;
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
            $items[] = l($type->label, SALESFORCE_PATH_DEMO .'/data-structure/'. $type->name);
          }
          $output .= theme('item_list', $items, t('Global Data Structure'));
        }
      }

      break;
  }
  
  $items = array(
    l(t('Examine Data Structure'), SALESFORCE_PATH_DEMO .'/data-structure'));
  if (module_exists('sf_user')) {
    $items[] = l(t('Export your user account as a contact'), 'user/' . $user->uid . '/salesforce');
  }
  if (module_exists('sf_node') && $node = node_load(array('type' => 'page'))) {
    $items[] = l(t('Export node @nid as a Campaign', array('@nid' => $node->nid)), 'node/' . $node->nid . '/salesforce');
  }

  $output .= '<p>'. t('<b>Choose from the following demonstrations:</b>') . theme('item_list', $items) .'</p>';

  return $output;
}



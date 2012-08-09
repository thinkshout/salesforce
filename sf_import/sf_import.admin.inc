<?php

/**
 * @file
 * Admin settings for the SF Import module.
 */

/**
 * Import Settings form.
 */
function sf_import_settings_form($form_state) {
  $form = array();

  $form['overview'] = array(
    '#value' => 'Ongoing imports from Salesforce are possible by configuring the fieldmap(s) defined below. On each cron run, Salesforce Import will check for updated records and import them into Drupal.',
    '#prefix' => '<p>',
    '#suffix' => '</p>',
  );

  // Show the time of last import
  $last_import = variable_get('sf_import_queue_last_import', NULL);
  if ($last_import) {
    $date = date('M j Y H:i:s', $last_import);
    // Get the number of items remaining in the import queue
    $num_items = db_result(db_query("SELECT COUNT(sfid) FROM {sf_import_queue}"));

    $data = '<strong>Last import: </strong>' . $date . ' <br /><strong>Number of items imported:</strong> ' .
      variable_get('sf_import_queue_import_count', 0) . '<br /><strong>Number of items processed:</strong> ' .
      variable_get('sf_import_queue_processed_count', 0) . '<br /><strong>Number of items remaining in queue:</strong> ' .
      $num_items;

    $form['last_import'] = array(
      '#value' => $data,
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    );
  }

  // Load the fieldmaps
  $fieldmaps = salesforce_api_salesforce_field_map_load_all();
  $maps = array();
  foreach ($fieldmaps as $map) {
    $maps[$map->name] = $map->name;
    if ($map->description) {
      $maps[$map->name] .= '<em> (' . $map->description . ')</em>';
    }
  }

  $form['sf_import_fieldmaps'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Fieldmaps'),
    '#description' => t('Select which fieldmaps should be used for ongoing imports from Salesforce to Drupal.'),
    '#options' => $maps,
    '#default_value' => variable_get('sf_import_fieldmaps', array()),
  );

  // Advanced settings
  $form['sf_import_advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $import_maps = variable_get('sf_import_fieldmaps', array());
  $active_import_maps = array();
  if ($import_maps) {
    foreach ($import_maps as $import_map_key => $import_map_value) {
      // Custom import settings
      $form['sf_import_advanced'][$import_map_key]['update_settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Import settings for fieldmap "@map"', array('@map' => (!empty($fieldmaps[$import_map_key]->description) ? $fieldmaps[$import_map_key]->description : $fieldmaps[$import_map_key]->name))),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $form['sf_import_advanced'][$import_map_key]['update_settings']['sf_import_' . $import_map_key . '_update_method'] = array(
        '#type' => 'radios',
        '#title' => 'Import method',
        '#description' => 'Select the method for selecting Salesforce IDs to import.',
        '#options' => array('soql' => 'Custom SOQL query', 'get_updated' => 'Salesforce API getUpdated() method'),
        '#default_value' => variable_get('sf_import_' . $import_map_key . '_update_method', 'get_updated'),
      );
      $form['sf_import_advanced'][$import_map_key]['update_settings']['default_soql'] = array(
        '#value' => 'Default SOQL statement used for importing data: <code>SELECT Id, LastModifiedDate FROM ' . $fieldmaps[$import_map_key]->salesforce . ' WHERE LastModifiedDate > ' . gmdate(DATE_ATOM, variable_get('sf_import_queue_last_import', time())) . '</code>',
        '#prefix' => '<p>',
        '#suffix' => '</p>',
      );
      $form['sf_import_advanced'][$import_map_key]['update_settings']['sf_import_' . $import_map_key . '_soql'] = array(
        '#type' => 'textarea',
        '#title' => 'SOQL query',
        '#description' => t('Enter any additional SOQL WHERE clauses for querying Salesforce about updated records in the format of "Email != NULL AND FirstName != NULL".'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#default_value' => variable_get('sf_import_' . $import_map_key . '_soql', NULL),
      );
    }
  }

  $form['sf_import_advanced']['sf_import_cron'] = array(
    '#type' => 'fieldset',
    '#title' => t('Cron'),
    '#description' => t('You should only uncheck the boxes below if you intend to implement your own import and processing functions.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['sf_import_advanced']['sf_import_cron']['sf_import_cron_import'] = array(
    '#type' => 'checkbox',
    '#title' => t('On cron run, import updated SFIDs to the {sf_import_queue} table.'),
    '#default_value' => variable_get('sf_import_cron_import', 1),
  );

  $form['sf_import_advanced']['sf_import_cron']['sf_import_cron_process'] = array(
    '#type' => 'checkbox',
    '#title' => t('On cron run, process SFIDs in the Salesforce import queue.'),
    '#default_value' => variable_get('sf_import_cron_process', 1),
  );


  // For fieldmaps that have import enabled, show some information about them
  $import_maps = variable_get('sf_import_fieldmaps', array());
  $active_import_maps = array();
  if ($import_maps) {
    foreach ($import_maps as $import_map_key => $import_map_value) {
      if ($import_map_value !== 0) {
        $map = salesforce_api_salesforce_field_map_load($import_map_key);
        $start = variable_get('sf_import_queue_last_import', time());
        $end = time();
        // salesforce_api_get_updated requires the query window to be at least 1 minute
        if ($end - $start < 60) {
          $start = $end - 61;
        }
        $import_method = variable_get('sf_import_' . $map->name . '_update_method', 'get_updated');
        $import_function = ($import_method == 'get_updated') ? 'salesforce_api_get_updated' : '_sf_import_get_soql_records';
        $sf_updated = $import_function($map->salesforce, $start, $end);

        $form[$import_map_key . '_information'] = array(
          '#type' => 'fieldset',
          '#title' => t('Information for fieldmap "@map"', array('@map' => (!empty($map->description) ? $map->description : $import_map_key))),
          '#description' => t('Number of updated records and timestamp of last update, if information is available.'),
          '#collapsible' => TRUE,
          '#collapsed' => $sf_updated ? FALSE : TRUE,
        );

        if ($sf_updated) {
          $form[$import_map_key . '_information']['pending'] = array(
            '#markup' => t('@count updated record(s) in Salesforce pending import.<br />Newest object in Salesforce dated @date', array('@count' => count($sf_updated->ids), '@date' => $sf_updated->latestDateCovered)),
            '#prefix' => '<p>',
            '#suffix' => '</p>',
          );
          $active_import_maps[$import_map_key] = $import_map_value;
          // Show table of SFIDs pending import
          $rows = array();
          $header = array('Salesforce ID');
          $sf_data = $sf_updated->ids;

          foreach ($sf_data as $key => $sfid) {
            $rows[] = array($sfid);
          }

          $form[$import_map_key . '_information']['data'] = array(
            '#markup' => theme_table($header, $rows),
            '#prefix' => '<div>',
            '#suffix' => '</div>',
          );
        }
        else {
          $form[$import_map_key . '_information']['nodata'] = array(
            '#markup' => 'There are no pending updates for this fieldmap.',
            '#prefix' => '<p>',
            '#suffix' => '</p>',
          );
        }
      }
    }

    // Allow the user to import records for fieldmaps that have updates
    if ($active_import_maps) {
      $form['process_updates'] = array(
        '#type' => 'fieldset',
        '#title' => t('Process updates for all fieldmaps'),
        '#description' => t('Query Salesforce for updates for all fieldmaps and import updated data.'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      );

      $form['process_updates']['get_updates'] = array(
        '#type' => 'submit',
        '#value' => t('Import updates'),
      );
      $form['#submit'][] = 'sf_import_settings_form_submit';
    }
  }
  return system_settings_form($form);
}

/**
 * Submit handler for the settings page.
 */
function sf_import_settings_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == 'Import updates') {
    if (variable_get('sf_import_fieldmaps', array())) {
      $updates = sf_import_import_records();
      $processed = _sf_import_process_records();
      if (user_access('administer salesforce')) {
        if ($updates) {
          drupal_set_message(t('@updates record(s) imported from Salesforce.'), array('@updates' => count($updates)));
        }
        if ($processed) {
          drupal_set_message(t('@processed imported record(s) processed.'), array('@processed' => count($processed)));
        }
        drupal_set_message(t('Please allow one minute to refresh this page for accurate information about newly updated records in Salesforce.'));
      }
    }
  }
}

/**
 * Page callback for admin/settings/salesforce/import/create
 */
function sf_import_create(&$form_state, $ongoing = 0) {

  $form = $options = array();

  $fieldmaps = salesforce_api_salesforce_field_map_load_all();
  foreach ($fieldmaps as $map) {
    $edit = l('edit', SALESFORCE_PATH_FIELDMAPS . '/' . $map->name . '/edit', array('query' => array('destination' => drupal_get_destination())));
    $options[$map->name] =
      salesforce_api_fieldmap_object_label('salesforce', $map->salesforce) . ' => ' .
      salesforce_api_fieldmap_object_label('drupal', $map->drupal) . ' - <em>' . $map->description . '</em>' . ' (' . $edit . ')';
  }

  // Add a message if no objects have been mapped.
  if (empty($options)) {
    drupal_set_message(t('You have not yet defined any fieldmaps.'), 'error');
    return;
  }

  // Admin should select a mapping to use for the import.
  $form['label'] = array(
      '#type' => 'markup',
      '#value' => '<h2>' . ($ongoing ? t('Create Ongoing Import') : t('Perform One-time Import')) . '</h2>',
  );

  $form['fieldmap'] = array(
      '#title' => t('Please choose a fieldmap to use for the import'),
      '#description' => t('Salesforce Object => Drupal Content Type'),
      '#type' => 'radios',
      '#required' => TRUE,
      '#options' => $options,
  );

  $form['extra-options'] = array(
      '#title' => t('Extra Options'),
      '#type' => 'fieldset',
      '#collasible' => FALSE,
      '#collapsed' => FALSE,
    );

  $form['extra-options']['extra-linked'] = array(
      '#title' => t('Link nodes to Salesforce objects on import?'),
      '#description' => t('Links the imported Drupal node to the salesforce object allowing the ability to issue manual syncronization of data to and from Drupal and Salesforce business objects. Linking also enables the ability to use node reference to relate business objects in Drupal (like accounts to contacts).'),
      '#type' => 'checkbox',
  );

  $form['extra-options']['extra-where'] = array(
      '#title' => t('Conditions'),
      '#description' => t("<strong>Advanced</strong>: Enter any additional SOQL \"Where\" conditions to use for this import query, e.g.<br /><code>Type != 'One-to-One Individual'</code><br />Learn more here: <a href='http://www.salesforce.com/us/developer/docs/api/Content/sforce_api_calls_soql_select_conditionexpression.htm' target='_blank'>Salesforce.com SOQL Where clause</a>"),
      '#type' => 'textarea',
  );

  $form['ongoing'] = array('#type' => 'value', '#value' => $ongoing);
  $form['submit'] = array('#type' => 'submit', '#value' => 'Submit');

  return $form;
}

/**
 * Submit callback for sf_import_create form.
 *
 * @param array $form
 * @param array $form_state
 * @param int $ongoing
 */
function sf_import_create_submit($form, &$form_state, $ongoing = 0) {
  // Create a new batch job to do the import
  module_load_include('inc', 'sf_import', 'sf_import.batch');
  $extra_options = array();
  $extra_options['extra-linked'] = $form_state['values']['extra-linked'];
  $extra_options['extra-where'] = $form_state['values']['extra-where'];
  $batch = sf_import_create_batchjob($form_state['values']['fieldmap'], $extra_options);
  batch_set($batch);
}

/**
 * Page callback for Manual imports. Allow a user to specify a list of
 * Salesforce IDs for importing from Salesforce to Drupal.
 *
 * @param array &$form_state
 */
function sf_import_manual(&$form_state) {
  $form = array();
  $fieldmaps = salesforce_api_salesforce_field_map_load_all();
  $maps = array();
  foreach ($fieldmaps as $map) {
    $maps[$map->name] = !empty($map->description) ? $map->description : $map->name;
  }

  $form['fieldmap'] = array(
    '#type' => 'select',
    '#title' => t('Fieldmap to use for import'),
    '#options' => $maps,
    '#required' => TRUE,
    '#description' => t('Select a fieldmap to use for importing the SFIDs to Drupal.'),
  );

  $form['sfids'] = array(
    '#title' => t('Salesforce IDs'),
    '#type' => 'textarea',
    '#required' => TRUE,
    '#description' => t('Enter a list of Salesforce IDs, one per line, that you would like to import from Salesforce to Drupal.'),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => 'Import');

  return $form;
}

/**
 * Validation callback for sf_import_manual.
 * Checks to see if the SFIDs appear to be in a valid format using the is_sfid()
 * function.
 */
function sf_import_manual_validate($form, &$form_state) {
  $sfid_data = $form_state['values']['sfids'];
  $sfids = explode("\n", $sfid_data);
  $errors = array();
  // Check for errors, and run sfids through rtrim() to remove
  // trailing spaces.
  foreach ($sfids as &$sfid) {
    $sfid = rtrim($sfid);
    if (!empty($sfid) && !is_sfid($sfid)) {
      $errors[] = $sfid;
    }
  }
  if ($errors) {
    form_set_error('sfids', t('The following SFID(s) do not appear to be valid: @errors.', array('@errors' => implode(', ', $errors))));
  }
  $form_state['storage']['sfids'] = $sfids;
  $form_state['storage']['fieldmap'] = $form_state['values']['fieldmap'];
}

/**
 * Submit call back for sf_import_manual.
 */
function sf_import_manual_submit($form, &$form_state) {
  $sfids = $form_state['storage']['sfids'];
  $map = $form_state['storage']['fieldmap'];
  $fieldmap = salesforce_api_salesforce_field_map_load($map);
  $function = 'sf_' . $fieldmap->drupal . '_import';
  if (function_exists($function) && $fieldmap->name) {
    $errors = array();
    $successes = array();
    foreach ($sfids as $sfid) {
      if ($uid = $function($sfid, $fieldmap->name)) {
        if (user_access('administer salesforce')) {
          drupal_set_message(t('Succesfully linked SFID @sfid with user !user.', array('@sfid' => $sfid, '!user' => $uid)));
        }
      } else {
        if (user_access('administer salesforce')) {
          drupal_set_message(t('Failed to import SFID @sfid', array('@sfid' => $sfid)), 'error');
        }
      }
    }
  }
}

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

  $form['sf_import_advanced']['sf_import_cron'] = array (
    '#type' => 'fieldset',
    '#title' => t('Cron'),
    '#description' => t('You should only uncheck the boxes below if you intend to implement your own import and processing functions.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['sf_import_advanced']['sf_import_cron']['sf_import_cron_import'] = array(
    '#type' => 'checkbox',
    '#title' => t('On cron run, import SFIDs returned by the Salesforce API <code>get_updated()</code> function.'),
    '#default_value' => variable_get('sf_import_cron_import', 1),
  );

  $form['sf_import_advanced']['sf_import_cron']['sf_import_cron_process'] = array(
    '#type' => 'checkbox',
    '#title' => t('On cron run, run import operations for SFIDs in the Salesforce import queue.'),
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
        $sf_updated = salesforce_api_get_updated($map->salesforce, $start, $end);

        $form[$import_map_key . '_information'] = array(
          '#type' => 'fieldset',
          '#title' => t('Information for fieldmap <em>' . check_plain($import_map_key) . '</em>'),
          '#description' => t('Number of updated records and timestamp of last update, if information is available.'),
          '#collapsible' => TRUE,
          '#collapsed' => $sf_updated ? FALSE : TRUE,
        );
        if ($sf_updated) {
          $form[$import_map_key . '_information']['pending'] = array(
            '#value' => '<em>' . count($sf_updated->ids) . '</em> updated record(s) in Salesforce pending import.<br />Newest object in Salesforce dated <em>' . $sf_updated->latestDateCovered . '.</em>',
            '#prefix' => '<div>',
            '#suffix' => '</div>',
          );
          $active_import_maps[$import_map_key] = $import_map_value;
          // Show table of SFIDs pending import
          $rows = array();
          $header = array('Salesforce ID');
          $sf_data = $sf_updated->ids;

          foreach ($sf_data as $key => $sfid) {
            $rows[] = array($sfid);
          }

          $form[$import_map_key . '_information']['pending']['data'] = array(
            '#value' => theme_table($header, $rows),
            '#prefix' => '<div>',
            '#suffix' => '</div>',
          );
        }
        else {
          $form[$import_map_key . '_information']['nodata'] = array(
            '#value' => 'There are no pending updates for this fieldmap.',
            '#prefix' => '<div>',
            '#suffix' => '</div>',
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
      $updates = _sf_import_get_updated_records();
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

  $extra_options = array();
  $extra_options['extra-linked'] = $form_state['values']['extra-linked'];
  $extra_options['extra-where'] = $form_state['values']['extra-where'];

  $batch = sf_import_create_batchjob($form_state['values']['fieldmap'], $extra_options);
  batch_set($batch);
}

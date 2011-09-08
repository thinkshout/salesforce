<?php

/**
 * @file
 */

// @todo add tracking and reporting of messages received from Salesforce

/**
 * Salesforce Notifications settings
 *
 * @todo Response handling: under what conditions should we return a successful response?
 *       Note if we do not return a successful response, Salesforce will re-send the outbound
 *       message indefinitely.
 */
function sf_notifications_settings_form($form, &$form_state) {
  $form = array('sf_notifications' => array(
      '#type' => 'fieldset',
      '#title' => 'Active Notification Fieldmaps',
      '#description' => 'Please check the box for each fieldmap you would like to respond to Salesforce Outbound Message events.',
      'sf_notifications_active_maps' => array(
        '#type' => 'checkboxes',
        '#default_value' => variable_get('sf_notifications_active_maps', array()),
      ),
    ));
  $maps = salesforce_api_salesforce_fieldmap_load_all();
  foreach ($maps as $id => $map) {
    $edit = l(t('edit'), SALESFORCE_PATH_FIELDMAPS . '/' . $id . '/edit', array('query' => array('destination' => drupal_get_destination())));
    $form['sf_notifications']['sf_notifications_active_maps']['#options'][$id] = t('@drupal => @salesforce - %description (!edit)', array('@drupal' => $map->drupal_entity . ' : ' . $map->drupal_bundle, '@salesforce' => $map->salesforce, '%description' => $map->description, '!edit' => $edit));
  }

  // IP Whitelist form
  $form['sf_notifications_ip_whitelist'] = array(
    '#type' => 'fieldset',
    '#title' => t('IP Whitelist'),
    '#description' => t('Settings for an IP whitelist of valid Salesforce IPs.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#access' => user_access('administer salesforce'),
  );

  $form['sf_notifications_ip_whitelist']['sf_notifications_allowed_ips'] = array(
    '#type' => 'textarea',
    '#title' => t('Allowed IPs'),
    '#description' => t('Enter the <a href="@IPs">Salesforce IP addresses</a> that will send outbound messages to your site. Enter one IP address or CIDR mask per line.', array('@IPs' => url('https://help.salesforce.com/apex/HTViewSolution?id=102757&language=en'))),
    '#cols' => 60,
    '#rows' => 5,
    '#default_value' => variable_get('sf_notifications_allowed_ips', implode("\n", sf_notifications_default_allowed_ips())),
  );


  // Queue Settings form
  $queue = DrupalQueue::get('sf_notifications_queue');

  $form['sf_notifications_queue'] = array(
    '#type' => 'fieldset',
    '#title' => t('Incoming Notifications Queue'),
    '#description' => t('Settings for the queueing of incoming notifications.<br/> There are @count items currently in the queue. !link', array('@count' => $queue->numberOfItems(), '!link' => l(t('View and manage the queue'), SALESFORCE_PATH_NOTIFICATIONS_ADMIN . '/queue'))),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#access' => user_access('administer salesforce'),
  );

  $form['sf_notifications_queue']['sf_notifications_use_queue'] = array(
    '#type' => 'checkbox',
    '#title' => t('Queue incoming salesforce messages.'),
    '#default_value' => variable_get('sf_notifications_use_queue', 0),
    '#description' => t('By placing incoming notifications into a queue, your site can stand up to the odd time when Salesforce sends a large volume of outbound notifications at once. However, the incoming messages will not be processed immediately.'),
  );

  $form['sf_notifications_queue']['sf_notifications_use_drupal_cron'] = array(
    '#type' => 'checkbox',
    '#title' => t('Process messages on Drupal cron.'),
    '#default_value' => variable_get('sf_notifications_use_drupal_cron', 1),
    '#description' => t('This checkbox has no effect it the queue is not enabled above. If this box is checked, the queue will be processed as part of the normal Drupal cron. However, if you want your messages processed on a different schedule, you can create a system cron job to execute the sf-process-notifications drush task. Note: The user executing the cron job / drush task must have read access to your AES module keyfile.'),
  );

  return system_settings_form($form);
}

/**
 * Salesforce Notifications queue form.
 *
 * Shows queue contents, and adds some controls to process and empty the queue.
 */
function sf_notifications_queue_form($form, &$form_state) {
  $queue = DrupalQueue::get('sf_notifications_queue');
  $form = array();

  $form['queue_count'] = array(
    '#markup' => t('There are currently @count items in the queue', array('@count' => $queue->numberOfItems())),
  );

  // get a list of items from the queue.
  $queue_items = array();
  $results = db_query("SELECT item_id, data, created FROM {queue} WHERE name = 'sf_notifications_queue'");
  foreach ($results as $item) {
    $data = unserialize($item->data);
    $queue_items[] = array($item->item_id, print_r($data, TRUE), format_date($item->created));
  }

  $header = array(array('data' => 'Queue ID', 'field' => 'item_id'), 'Data', array('data' => 'Created', 'field' => 'created'));

  $form['queue_table'] = array(
    '#markup' => theme('table', array('header' => $header, 'rows' => $queue_items, 'empty' => t('There are no items in the inbound notifications queue at the moment'))),
  );

  $form['actions']['#type'] = 'actions';

  $form['actions']['help_text'] = array(
    '#markup' => '<p>' . t("The buttons below only act on items in the queue that are not currently claimed for processing. If items still remain in the queue after clicking 'Empty Notifications Queue', please wait a minute for the queue claim to expire, then try again.") . '</p>',
  );

  $form['actions']['process'] = array(
    '#type' => 'submit',
    '#value' => t('Process queue items'),
//    '#disabled' => !$queue->numberOfItems() ? TRUE : FALSE,
    '#submit' => array('sf_notifications_process_submit'),
  );

  $form['actions']['empty'] = array(
    '#type' => 'submit',
    '#value' => t('Empty notifications queue'),
//    '#disabled' => !$queue->numberOfItems() ? TRUE : FALSE,
    '#submit' => array('sf_notifications_empty_submit'),
  );

  return $form;
}

// Submit handler for the 'Process queue items' button.
function sf_notifications_process_submit($form, &$form_state) {
  drupal_goto(SALESFORCE_PATH_NOTIFICATIONS_ADMIN . '/queue/process');
}

// Submit handler for the 'Empty queue items' button.
function sf_notifications_empty_submit($form, &$form_state) {
  drupal_goto(SALESFORCE_PATH_NOTIFICATIONS_ADMIN . '/queue/empty');
}

// Confirm form 'Process queue items' button.
function sf_notifications_process_confirm_form($form, &$form_state) {
  $form = array();

  // Set some default form values
  $form = confirm_form(
    $form,
    t('Are you sure you want to process the Salesforce notifications queue?'),
    SALESFORCE_PATH_NOTIFICATIONS_ADMIN . '/queue',
    NULL,
    t('Continue'),
    t('Cancel'));
  return $form;
}

// Submit handler for the 'Process queue items' button confirmation form.
function sf_notifications_process_confirm_form_submit($form, &$form_state) {
  // Release any expired claims so they are available to be re-claimed.
  sf_notifications_release_expired_claims();

  $queue = DrupalQueue::get('sf_notifications_queue');

  while ($item = $queue->claimItem(60)) {
    $ret = _sf_notifications_parse_handle_message($item->data);
    if ($ret) {
      salesforce_api_log(SALESFORCE_LOG_ALL, 'Queued notification processed. Contents: <pre>%content</pre>', array('%content' => print_r($item->data, TRUE)));
      $queue->deleteItem($item);
    }
    else {
      salesforce_api_log(SALESFORCE_LOG_ALL, 'Queued notification processing failed. Contents: <pre>%content</pre>', array('%content' => print_r($item->data, TRUE)), WATCHDOG_ERROR);
    }
  }

  drupal_set_message(t('Salesforce notifications queue processed. Check watchdog for any failed processing attempts.'));
  drupal_goto(SALESFORCE_PATH_NOTIFICATIONS_ADMIN . '/queue');
}

// Confirm form 'Empty queue items' button.
function sf_notifications_empty_confirm_form($form, &$form_state) {
  $form = array();

  // Set some default form values
  $form = confirm_form(
    $form,
    t('Are you sure you want to empty the Salesforce notifications queue?'),
    SALESFORCE_PATH_NOTIFICATIONS_ADMIN . '/queue',
    NULL,
    t('Continue'),
    t('Cancel'));

  return $form;
}

// Submit handler for the 'Empty queue items' button confirmation form.
function sf_notifications_empty_confirm_form_submit($form, &$form_state) {

  // Release any expired claims so they are available to be re-claimed.
  sf_notifications_release_expired_claims();

  $queue = DrupalQueue::get('sf_notifications_queue');
  while ($item = $queue->claimItem(60)) {
      $queue->deleteItem($item);
  }

  drupal_set_message(t('Salesforce notifications queue items deleted.'));
  drupal_goto(SALESFORCE_PATH_NOTIFICATIONS_ADMIN . '/queue');
}
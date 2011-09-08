<?php

/**
 * @file
 *   drush integration for apachesolr.
 */

/**
 * Implements hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * @see drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function sf_notifications_drush_command() {
  $items = array();

  // the key in the $items array is the name of the command.
  $items['sf-process-notifications'] = array(
    // the name of the function implementing your command.
    'callback' => 'sf_notifications_drush_process_queue',
    // a short description of your command
    'description' => dt('Processes the sf_notifications queue.'),
  );
  return $items;
}

/**
 * Implements hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function sf_notifications_drush_help($section) {
  switch ($section) {
    case 'drush:sf-process-notifications':
      return dt("Processes the sf_notifications queue to create Drupal objects from queued inbound Salesforce notifications.");
  }
}

/**
 * Drush callback to process the Salesforce queue manually.
 */
function sf_notifications_drush_process_queue() {
  $total = 0;
  $queue = DrupalQueue::get('sf_notifications_queue');
  while ($item = $queue->claimItem(60)) {
    $ret = _sf_notifications_parse_handle_message($item->data);
    if ($ret) {
      salesforce_api_log(SALESFORCE_LOG_SOME, 'Queued notification processed. Contents: <pre>%content</pre>', array('%content' => print_r($item->data, TRUE)));
      drush_print(dt('Queued notification processed. Contents: <pre>%content</pre>', array('%content' => print_r($item->data, TRUE))));
      $queue->deleteItem($item);
      $total++;
    }
    else {
      salesforce_api_log(SALESFORCE_LOG_ALL, 'Queued notification processing failed. Contents: <pre>%content</pre>', array('%content' => print_r($item->data, TRUE)), WATCHDOG_ERROR);
      drush_print(dt('Queued notification processing failed. Contents: <pre>%content</pre>', array('%content' => print_r($item->data, TRUE))));
      $total++;
    }
  }

  drush_print(dt('@total items processed', array('@total' => $total)));
}

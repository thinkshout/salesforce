<?php

/**
 * @file
 * Report callbacks for the queue processor module.
 */

/**
 * Current queue report
 */
function current_queue_report() {
  $pager_num = 0;

  // @todo: Test this is working correctly.
  $rows = array();
  $query = db_select('sf_queue', 'sfq')
             ->fields('sfq', array('oid', 'sfid', 'type', 'action', 'created'))
             ->orderBy('created', 'DESC')
             ->extend('PagerDefault')
             ->limit(20);
  $result = $query->execute();

  // Loop over each item in queue and allow individual modules a chance to add the title.
  while($row = $result->fetchAssoc()) {
    $row['title'] = 'Unknown';
    $row['created'] = format_date($row['created'], 'large');
    drupal_alter('queue_report_item_title', $row);
    $rows[] = $row;
  }
  
  if(!empty($rows)) {
    $header = array('Drupal Id', 'Title', 'SFID', 'Type', 'Action', 'Date Added');
    $output = 'These items are currently queued up and will be exported to Salesforce during the next cron run.';
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'current-queue')));
    $output .= theme('pager', array('tags' => NULL, 'element' => $pager_num));
  }
  else {
    $output = '<p>' . t('The export queue is currently empty.') . '</p>';
  }
  return $output;
}

/**
 * List of batches that have been processed by the queue.
 */
function batch_history_report() {
  $pager_num = 0;
  
  // @todo: Test this is working correctly.
  $rows = array();
  $query = db_select('sf_batch', 'sfb')
             ->fields('sfb', array('id', 'type', 'created', 'record_count', 'success_count', 'failure_count', 'start_time', 'end_time'))
             ->orderBy('created', 'DESC')
             ->extend('PagerDefault')
             ->limit(20);
  $result = $query->execute();

  while ($row = $result->fetchAssoc()) {
    $row['id'] = l($row['id'], 'admin/reports/salesforce/batch/' . $row['id']);
    $row['created'] = format_date($row['created'], 'large');
    $row['start_time'] = format_date($row['start_time'], 'large');
    $row['end_time'] = format_date($row['end_time'], 'large');
    $rows[] = $row;
  }
  
  if(!empty($rows)) {
    $header = array('Id', 'Type', 'Created', 'Items', 'Successes', 'Failures', 'Start Time', 'End Time');
    $output = 'Click on the id of the batch to drill down into more detail.';
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'batch-report')));
    $output .= theme('pager', array('tags' => NULL, 'element' => $pager_num));
  }
  else {
    $output = '<p>' . t('There are no current batches.') . '</p>';
  }
  return $output;
}

/**
 * Batch detail page.
 */
function batch_detail($id) {

  $base = drupal_get_path('module', 'sf_queue_processor');
  drupal_add_js($base . '/sf_queue_processor.js');
  drupal_add_css($base . '/sf_queue_processor.css');

  //$output = 'Batch detail ' . $id . '<pre>';
  $batch = load_batch($id);

  //$output .= '</pre>';
  $output = '<div id="batch-details">';
  $output .= '<h2>Batch ' . $batch['id'] . '</h2>';
  $output .= 'Action: ' . $batch['action'] . '<br/>';
  $output .= 'Drupal Type: <span class="object-type">' . ucfirst($batch['type']) . '</span><br/>';
  $output .= 'Salesforce Type: <span class="object-type">' . ucfirst($batch['salesforce_type']) . '</span><br/>';
  $output .= '<h4>Items</h4>';
  $output .= '</div>';

  //$output .= '<pre>' . print_r($batch, true) . '</pre>';


  foreach ($batch['records'] as $item) {
    $rows = array();
    $object = unserialize($item->data);
    if ($batch['action'] != 'delete') {
      foreach ($object as $field => $value) {
        $rows[] = array(
          'field' => $field,
          'value' => $value,
        );
      }
    }

    asort($rows);
    $retry = ($item->is_retry == 1) ? 'Yes' : 'No';
    $success = ($item->success == 1) ? 'Yes' : 'No';
    $output .= '<div id="batch-item-container">';
    $output .= '<h5>' . ucfirst($batch['type']) . ' ' . $item->oid . '</h5>';
    $output .= '<div class="batch-item">';
    $output .= '<span>Success:</span>' . $success . '<br/>';

    if (!$item->success) {
      $error = load_batch_error($batch['id'], $item->oid, $batch['type']);
      $output .= '<div class="item-error">';
      $output .= '<span>Error message:</span>' . $error->message . '<br/>';
      $output .= '<span>Status code:</span>' . $error->status_code . '<br/>';
      $output .= '<span>Fields:</span>' . $error->fields . '<br/>';
      if ($url = created_edit_link($item->oid, $item->type, 'admin/reports/salesforce/batch/' . $batch['id'])) {
        $output .= l(t('Click to edit object'), $url);
      }
      $output .= '</div>';
    }

    $output .= '<span>Processed:</span>' . date('n/j/Y g:i:s a', $item->date_processed) . '<br/>';
    $output .= '<span>Was retry:</span>' . $retry . '<br/>';
    $output .= '<a href="#" class="object-details">Click for object details</a><br />';
    $output .= '<div class="batch-item-fields">';
    $output .= theme('table', array('header' => array('Field', 'Value'), 'rows' => $rows));
    $output .= '</div>';
    $output .= '</div>';
    $output .= '</div>';
    //$output .= '</div>';
  }

  return $output;
}

/**
 * Salesforce reports landing page.
 */
function salesforce_integration_reports() {
  $output = 'See various reports on the status of your Salesforce integration.';
  $items = array(
    l('Batch history', 'admin/reports/salesforce/batch'),
    l('Currently queued items', 'admin/reports/salesforce/current'),
    l('Retry queue', 'admin/reports/salesforce/retries'),
    l('Permanent failures', 'admin/reports/salesforce/permanent-failures'),
  );
  $output .= theme('item_list', array('items' => $items));
  return $output;
}

/**
 * Lists all items in the retry queue.
 */
function retry_queue_report() {
  $output = 'The following items are currently in the retry queue. If not edited beforehand, the next export attempt will be performed on the items\'s retry date.';
  $pager_num = 0;
  
  $rows = array();
  $query = db_select('sf_retry_queue', 'sfrq')
             ->fields('sfrq', array('oid', 'sfid', 'type', 'name', 'action', 'retry_date', 'attempts'))
             ->orderBy('retry_date', 'ASC')
             ->extend('PagerDefault')
             ->limit(20);
  $result = $query->execute();

  while ($row = $result->fetchAssoc()) {
    $row['retry_date'] = format_date($row['retry_date'], 'large');
    $rows[] = $row;
  }
  
  if(!empty($rows)) {
    $header = array('Drupal Id', 'SFID', 'Type', 'Fieldmap', 'Action', 'Retry Date', 'Attempts');
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'retry-queue')));
    $output .= theme('pager', array('tags' => NULL, 'element' => $pager_num));
  }
  else {
    $output = '<p>' . t('There are no items currently waiting to be retried.') . '</p>';
  }
  return $output;
}

/**
 * Lists all items that have been flagged as a permanent failure.
 */
function permanent_failures_report() {
  $output = 'The following items have been marked as permanent failures. No more attempts to export them will be made until they have been edited.';
  $pager_num = 0;
  
  $rows = array();
  $query = db_select('sf_permanent_failure', 'sfpf')
             ->fields('sfpf', array('oid', 'type', 'name', 'action', 'date_added'))
             ->orderBy('date_added', 'DESC')
             ->extend('PagerDefault')
             ->limit(20);
  $result = $query->execute();

  while ($row = $result->fetchAssoc()) {
    $row['date_added'] = format_date($row['date_added']);
    $rows[] = $row;
  }
  
  if(!empty($rows)) {
    $header = array('Drupal Id', 'Type', 'Fieldmap', 'Action', 'Date Added');
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'permanent-failures')));
    $output .= theme('pager', array('tags' => NULL, 'element' => $pager_num));
  }
  else {
    $output = '<p>' . t('There have been no permanent failures.') . '</p>';
  }
  return $output;
}

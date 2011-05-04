<?php
// $Id$

/**
 * @file
 */

function sf_queue_admin_form() {
  $sf_queue_count = db_result(db_query("SELECT count(created) FROM {salesforce_export_queue}"));
  $mapinfo = _sf_queue_admin_mapinfo();
  $header = array(t('Manage'), t('Operation'), t('Queue ID'), t('#Attempts'), t('Created'), t('Drupal Type'), t('SF Type'), t('Fieldmap'));
  $rows = array();

  $sf_queue_items = db_query("SELECT * FROM {salesforce_export_queue} ORDER BY created");

  while ($sf_queue_item = db_fetch_object($sf_queue_items)) {
    if (isset($mapinfo[$sf_queue_item->fieldmap_name])) {
      $fieldmap = $mapinfo[$sf_queue_item->fieldmap_name];
    }
    else {
      $fieldmap = $sf_queue_item->fieldmap_name;
    }
    $rows[] = array(
      'data' => array(
        l(t('View'), "admin/reports/sf_queue/view/" . $sf_queue_item->id) . ' ' . l(t('Delete'), "admin/reports/sf_queue/delete/" . $sf_queue_item->id),
        $sf_queue_item->sf_op,
        $sf_queue_item->id,
        $sf_queue_item->attempts,
        date("r", $sf_queue_item->created),
        $sf_queue_item->drupal_type,
        $sf_queue_item->sf_type,
        $fieldmap,
      ),
    );
  }

  $output = theme('table', $header, $rows);

  if ($sf_queue_count > 0) {
    if ($sf_queue_count == 1) {
      $output .= "There is 1 item in the queue.";
    }
    else {
      $output .= "There are " . count($rows) . " items in the export queue.";
    }
    $output .= "<br />" . l(t('Process queued items now'), "admin/reports/sf_queue/processqueue");
  }
  else {
    $output .= "There are no items in the export queue.";
  }

  return $output;
}

function  sf_queue_admin_view($qid) {
  if (strlen($qid) == 0 || !is_numeric($qid)) {
    drupal_goto("admin/reports/sf_queue/");
  }

  $sf_queue_item = db_fetch_object(db_query("SELECT * FROM {salesforce_export_queue} WHERE id=" . $qid));

  $header = array(t('Field name'), t('Value'));
  $rows = array();

  foreach ($sf_queue_item as $key => $value) {
    if ($key == "sf_data") {
      $sf_data = unserialize($value);
      foreach ($sf_data as $key_sf => $value_sf) {
        $rows[] = array(
          'data' => array(
          "sf_data - " . $key_sf,
          $value_sf,
          ),
        );
      }
    }
    else {
      $rows[] = array(
        'data' => array(
          $key,
          $value,
        ),
      );
    }
  }

  if (count($rows) == 0) {
    drupal_goto("admin/reports/sf_queue/");
  }

  $output  = theme('table', $header, $rows);
  $output .= l(t('Delete this queue item'), "admin/reports/sf_queue/delete/" . $qid) . "<br />";
  $output .= l(t('Return to queue list'), "admin/reports/sf_queue");

  return $output;
}

function  sf_queue_admin_delete_confirm(&$form_state, $qid) {
  if (strlen($qid) == 0 || !is_numeric($qid)) {
    drupal_goto("admin/reports/sf_queue/");
  }

  $results = db_fetch_object(db_query("SELECT count(id) count FROM {salesforce_export_queue} WHERE id=" . $qid));
  if ($results->count == 0) {
    drupal_goto("admin/reports/sf_queue/");
  }

  $form = array();
  $form['_qid'] = array(
    '#type' => 'value',
    '#value' => $qid,
  );

  $question = t('Are you sure you want to delete the queue item number ' . $qid . '?');

  return confirm_form($form,
                      $question,
                      "admin/reports/sf_queue",
                      "<em>" . $question . "</em><br />" . t('This action cannot be undone.'),
                      t('Delete'),
                      t('Cancel')
                     );
}

function sf_queue_admin_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    db_query("DELETE from {salesforce_export_queue} WHERE id = '%s'", $form['_qid']['#value']);
    drupal_set_message(t('Queue item [' . $form['_qid']['#value'] . '] has been deleted successfully.'));
  }
  else {
    drupal_set_message(t('Queue item [' . $form['_qid']['#value'] . '] was not deleted.'));
  }

  drupal_goto("admin/reports/sf_queue/");
}


function  _sf_queue_admin_mapinfo() {
  $sf_maps_items = db_query("SELECT name,description FROM {salesforce_field_map}");
  while ($sf_map = db_fetch_object($sf_maps_items)) {
    $name = $sf_map->name;
    $description = $sf_map->description;
    $sf_mapinfo[$name] = $description;
  }

  if (!isset($sf_mapinfo)) {
    return NULL;
  }

  return $sf_mapinfo;
}

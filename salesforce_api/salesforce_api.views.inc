<?php

/**
 * @file
 * Expose local Salesforce data to views
 */

/**
 * Implements hook_views_data
 */
function salesforce_api_views_data() {
  $data['salesforce_object_map'] = array(
    'table' => array(
      'group' => 'Salesforce',
      'title' => 'salesforce_object_map',
      'join' => array(
        'node' => array(
          'left_field' => 'nid',
          'field' => 'oid',
          'extra' => array(
            array(
              'field' => 'drupal_type',
              'value' => 'node',
            ),
          ),
        ),
        'users' => array(
          'left_field' => 'uid',
          'field' => 'oid',
          'extra' => array(
            array(
              'field' => 'drupal_type',
              'value' => 'user',
            ),
          ),
        ),
      ),
    ),
    'sfid' => array(
      'title' => t('Salesforce Object ID'),
      'help' => t('The Object\'s Salesforce ID (if linked)'),
      'field' => array(
        'handler' => 'views_handler_field',
        'click sortable' => TRUE,
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_string',
        'allow empty' => 1,
      ),
    ),
  );

  return $data;
}

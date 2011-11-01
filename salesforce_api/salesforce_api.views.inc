<?php
// $Id$

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
    'created' => array(
      'title' => t('Timestamp of initial link between Drupal/Salesforce'),
      'help' => t('The timestamp when the link between Drupal and Salesforce was first created for this object.'),
      'field' => array(
        'handler' => 'views_handler_field_date',
        'click sortable' => TRUE,
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_date',
        'allow empty' => 1,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort_date',
      ),
    ),
    'last_export' => array(
      'title' => t('Timestamp of last export from Drupal to Salesforce'),
      'help' => t('The timestamp when the object was last exported from Drupal to Salesforce.'),
      'field' => array(
        'handler' => 'views_handler_field_date',
        'click sortable' => TRUE,
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_date',
        'allow empty' => 1,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort_date',
      ),
    ),
    'last_import' => array(
      'title' => t('Timestamp of last import from Salesforce to Drupal'),
      'help' => t('The timestamp when the object was last imported from Salesforce to Drupal.'),
      'field' => array(
        'handler' => 'views_handler_field_date',
        'click sortable' => TRUE,
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_date',
        'allow empty' => 1,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort_date',
      ),
    ),
  );

  return $data;
}

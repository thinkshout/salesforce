<?php
// $Id$

/**
 * @file
 */

// @todo add tracking and reporting of messages received from SalesForce

/**
 * SF Notifications settings
 *
 * @todo Response handling: under what conditions should we return a successful response? Note if we do not return a successful response, Salesforce will re-send the outbound message indefinitely.
 */
function sf_notifications_settings_form(&$form_state) {
  $form = array('sf_notifications' => array(
    '#type' => 'fieldset',
    '#title' => 'Active Notification Fieldmaps',
    '#description' => 'Please check the box for each fieldmap you would like to respond to Salesforce Outbound Message events.',
    'sf_notifications_active_maps' => array(
      '#type' => 'checkboxes',
      '#default_value' => variable_get('sf_notifications_active_maps', array()),
    )));
  $maps = salesforce_api_salesforce_field_map_load_all();
  foreach ($maps as $id => $map) {
    $edit = l(t('edit'), SALESFORCE_PATH_FIELDMAPS . '/' . $id . '/edit', array('query' => array('destination' => drupal_get_destination())));
    $form['sf_notifications']['sf_notifications_active_maps']['#options'][$id] = t('@drupal => @salesforce - %description (!edit)', array('@drupal' => $map->drupal, '@salesforce' => $map->salesforce, '%description' => $map->description, '!edit' => $edit));
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

  return system_settings_form($form);
}

function sf_notifications_fieldmap_settings(&$form_state, $fieldmap_id) {
  $map = salesforce_api_fieldmap_load($fieldmap_id);
  $active = variable_get('sf_notifications_active_maps', array());
  $active = $active[$fieldmap_id];
  $form = array(
    'active' => array(
      '#type' => 'checkbox',
      '#title' => 'Active',
      '#description' => 'Check this box if this fieldmap should respond to Salesforce Outbound Message events.',
      '#default_value' => $active,
    ),
  );

  $form['sf_notifications_settings'] = array(
    '#tree' => TRUE,
  );
  $settings = variable_get('sf_notifications_settings', array('condition' => array()));

  foreach ($settings as $id => $setting) {
    $form['sf_notifications_settings'][$id] = array('#type' => 'value', '#value' => $setting);
  }
  $form['sf_notifications_settings'][$fieldmap_id] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#title' => 'Conditions',
    '#descripiton' => 'You may assign conditions under which notifications will respond to Outbound Message events. Notifications failing these conditions will not trigger this fieldmap. This may be particularly useful, e.g. in the case of Salesforce objects that are mapped to multiple Drupal objects.',
  );
  $settings = $settings[$fieldmap_id];
  $form['sf_notifications_settings'][$fieldmap_id]['condition'] = array(
    '#type' => 'textarea',
    '#title' => FALSE,
    '#default_value' => $settings['condition'],
    '#description' => t('PHP code - <strong>do not include &lt;?php ?> tags</strong>. This code should return TRUE or FALSE to determine whether this fieldmap should be triggered on a relevant Outbound Message event. Available variables are $map (fieldmap object to be used to create the import), $notification_data (array of parsed object data from salesforce, keys are "oid" - drupal object id on update or empty on insert; "name" - machine name of fieldmap used for this operation; "fields" - object with the actual data from Salesforce), $operation ("insert", "update", or "delete")'),
  );
  return system_settings_form($form);
}

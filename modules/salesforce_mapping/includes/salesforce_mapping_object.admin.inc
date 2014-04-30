<?php

/**
 * Implements hook_form().
 */
function salesforce_mapping_object_form($form, &$form_state, $mapped_object = NULL) {
  $form = array();

  // Get list of mapped entity types.
  $mapped_entities = salesforce_mapping_get_mapped_entities();
  $entity_options = array();
  // Build an options array of entity types that have mappings.
  foreach ($mapped_entities as $type => $bundles) {
    $info = entity_get_info($type);
    $entity_options[$type] = $info['label'];
  }

  $form['entity_type'] = array(
    '#type' => 'select',
    '#title' => t('Entity type'),
    '#options' => $entity_options,
    '#default_value' => isset($mapped_object->entity_type) ? $mapped_object->entity_type : '',
    '#required' => TRUE,
  );

  $form['entity_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Drupal Entity Id'),
    '#default_value' => isset($mapped_object->entity_id) ? $mapped_object->entity_id : '',
    '#size' => 19,
    '#required' => TRUE,
  );

  $form['salesforce_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Salesforce Id'),
    '#default_value' => isset($mapped_object->salesforce_id) ? $mapped_object->salesforce_id : '',
    '#size' => 19,
    '#maxlength' => 18,
    '#required' => TRUE,
  );

  // @TODO options to trigger push or pull?

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => isset($mapped_object->salesforce_mapping_object_id) ? t('Update mapping') : t('Save mapping'),
    ),
    'delete_link' => array(
      '#markup' => isset($mapped_object->salesforce_mapping_object_id) ? l(t('Delete'), 'admin/structure/salesforce/mapped-objects/manage/' . $mapped_object->salesforce_mapping_object_id . '/delete', array('attributes' => array('id' => array('mapped_object-delete-' . $mapped_object->salesforce_mapping_object_id), 'class' => array('button remove')), 'query' => array('destination' => 'admin/structure/salesforce/mapped-objects'))) : ''));
  return $form;
}

/**
 * Implements hook_form_validate().
 */
function salesforce_mapping_object_form_validate($form, &$form_state) {
  $values = &$form_state['values'];

  $entity_type = $values['entity_type'];
  // Check that entity type + id is of a mapped type/bundle.
  $entity = entity_load_single($entity_type, $values['entity_id']);
  if (!$entity) {
    // Invalid entity
    form_set_error('entity_id', t('Entity is invalid.'));
  }
  else {
    // Check for existing mapping
    if (!empty($entity->salesforce_id)) {
      form_set_error('entity_id', t('Selected entity is already mapped to a Salesforce object.'));
    }
    $mapped_entities = salesforce_mapping_get_mapped_entities();
    list(, , $bundle) = entity_extract_ids($entity_type, $entity);
    if (empty($mapped_entities[$entity_type][$bundle]) && FALSE) {
      form_set_error('entity_id', t('Selected entity is of a bundle that is not allowed in any mappings.'));
    }
  }
  // Convert 15 character Salesforce Id to 18 characters.

  // Validate that Salesforce object is of an allowed type.
}

/**
 * Implements hook_form_submit().
 */
function salesforce_mapping_object_form_submit($form, &$form_state) {
  $mapped_object = entity_ui_form_submit_build_entity($form, $form_state);
  $mapped_object->save();
  drupal_set_message(t('Salesforce mapping has been saved.'));
  $form_state['redirect'] = 'admin/content/salesforce/mapped-objects';
}
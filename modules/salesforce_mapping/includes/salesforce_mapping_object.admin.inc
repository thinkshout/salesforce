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
    '#description' => t('15 character Ids will be treated as case sensitive and will be converted to their 18 character counterpart before saving.'),
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
  $mapping_object = $form_state['salesforce_mapping_object'];

  $entity_type = $values['entity_type'];
  $entity = entity_load_single($entity_type, $values['entity_id']);
  if (!$entity) {
    // Invalid entity
    form_set_error('entity_id', t('Entity is invalid.'));
  }
  else {
    list($entity_id, , $bundle) = entity_extract_ids($entity_type, $entity);

    // Check for existing mapping based on the Drupal entity.
    $mapping_object_drupal = salesforce_mapping_object_load_by_drupal($entity_type, $entity_id, TRUE);

    if ($mapping_object_drupal &&
      (empty($mapping_object->salesforce_mapping_object_id) || $mapping_object_drupal->salesforce_mapping_object_id != $mapping_object->salesforce_mapping_object_id)) {
      form_set_error('entity_id', t('Drupal entity is already mapped to a Salesforce object.'));
    }

    $mappings = salesforce_mapping_load_multiple(array(
      'drupal_entity_type' => $entity_type,
      'drupal_bundle' => $bundle,
    ));

    // Check that entity is of a mapped type/bundle.
    if (empty($mappings)) {
      form_set_error('entity_id', t('Selected entity is of a bundle that is not mapped.'));
    }
    else {
      $mapping = reset($mappings);
      $sf_object_type = $mapping->salesforce_object_type;

      $sfapi = salesforce_get_api();
      // Not authorized, we need to bail this time around.
      if (!$sfapi->isAuthorized()) {
        form_set_error('salesforce_id', t('Salesforce is not authorized.'));
      }

      // Attempt to retrieve the Salesforce object.
      $sf_object = FALSE;
      try {
        $sf_object = $sfapi->objectRead($sf_object_type, $values['salesforce_id']);
      }
      catch (Exception $e) {
        form_set_error('salesforce_id', t('Error getting object from Salesforce: @message', array('@message' => $e->getMessage())));
      }

      if ($sf_object) {
        // Use the returned Id to replace any 15 character Ids with the full 18 character one.
        $values['salesforce_id'] = $sf_object['Id'];

        // Check for existing mapping based on the Salesforce Id.
        $mapping_object_sf = salesforce_mapping_object_load_by_sfid($values['salesforce_id'], TRUE);

        if ($mapping_object_sf &&
          (empty($mapping_object->salesforce_mapping_object_id) || $mapping_object_sf->salesforce_mapping_object_id != $mapping_object->salesforce_mapping_object_id)) {
          form_set_error('salesforce_id', t('Salesforce object is already mapped to a Drupal entity.'));
        }

        // Validate that Salesforce object is of an allowed type and record type.
        $record_type = empty($sf_object['RecordTypeId']) ? 'default' : $sf_object['RecordTypeId'];

        $record_type_allowed = FALSE;
        foreach ($mappings as $mapping) {
          if (in_array($record_type, $mapping->salesforce_record_types_allowed, TRUE)) {
            $record_type_allowed = TRUE;
            continue;
          }
        }

        if (!$record_type_allowed) {
          form_set_error('salesforce_id', t('Salesforce object record type is not allowed in any mappings.'));
        }

      }
    }
  }

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
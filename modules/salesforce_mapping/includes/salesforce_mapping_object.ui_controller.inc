<?php
/**
 * Custom controller for the administrator UI.
 */
class SalesforceMappingObjectUIController extends EntityDefaultUIController {

  /**
   * Override the menu hook for default ui controller.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = t('Manage mapped Salesforce objects.');
    return $items;
  }

  /**
   * Admin form for searching and doing bulk operations.
   */
  public function overviewForm($form, &$form_state) {
    $form['pager'] = array('#theme' => 'pager');

    $header = array(
      'entity_type' => array(
        'data' => t('Entity type'),
        'type' => 'property',
        'specifier' => 'entity_type',
      ),
      'label' => array(
        'data' => t('Entity label'),
      ),
      'salesforce_id' => array(
        'data' => t('Salesforce object'),
        'type' => 'property',
        'specifier' => 'salesforce_id',
      ),
      'last_sync' => array(
        'data' => t('Last sync'),
        'type' => 'property',
        'specifier' => 'last_sync',
        'sort' => 'desc',
      ),
      'operations' => array('data' => t('Operations')),
    );

    $options = array();

    $query = new EntityFieldQuery();
    $query
      ->entityCondition('entity_type', 'salesforce_mapping_object');

    $query->tableSort($header);

    $query->pager(25);

    $result = $query->execute();
    $mapping_object_results = !empty($result['salesforce_mapping_object']) ? $result['salesforce_mapping_object'] : array();
    $mapping_object_array = !empty($mapping_object_results) ? entity_load('salesforce_mapping_object', array_keys($mapping_object_results)) : array();
    foreach ($mapping_object_array as $mapping_object_id => $mapping_object) {
      // Drupal entity.
      $drupal_entity = entity_load_single($mapping_object->entity_type, $mapping_object->entity_id);
      $uri = entity_uri($mapping_object->entity_type, $drupal_entity);
      $label = entity_label($mapping_object->entity_type, $drupal_entity);
      $drupal_entity_link = l($label, $uri['path'], $uri['options']);
      // Salesforce object id.
      // If we have a valid SF link, make the SF object id a link.
      $sfapi = salesforce_get_api();
      if ($sfapi) {
        $sf_object_link = l(
          $mapping_object->salesforce_id,
          $sfapi->getInstanceUrl() . '/' . $mapping_object->salesforce_id,
          array(
            'attributes' => array('target' => '_blank'),
          )
        );
      }
      else {
        $sf_object_link = $mapping_object->salesforce_id;
      }

      // Get human readable entity label
      $entity_info = entity_get_info($mapping_object->entity_type);
      $options['salesforce_mapping_object_id-' . $mapping_object_id] = array(
        'entity_type' => $entity_info['label'],
        'label' => $drupal_entity_link,
        'salesforce_id' => $sf_object_link,
        'last_sync' => format_date($mapping_object->last_sync),
        'operations' =>
          l(t('edit'), $this->path . '/manage/' . $mapping_object_id) . ' ' .
          l(t('delete'), $this->path . '/manage/' . $mapping_object_id . '/delete', array('query' => drupal_get_destination())),
      );
    }

    $form['search'] = array(
      '#type' => 'fieldset',
      '#title' => t('Search'),
      '#collapsible' => TRUE,
      '#collapsed' => !empty($search_term) ? FALSE : TRUE,
    );

    // Get list of mapped entity types.
    $mapped_entities = salesforce_mapping_get_mapped_entities();
    $entity_options = array('' => '');
    // Build an options array of entity types that have mappings.
    foreach ($mapped_entities as $type => $bundles) {
      $info = entity_get_info($type);
      $entity_options[$type] = $info['label'];
    }

    $form['search']['entity_type'] = array(
      '#type' => 'select',
      '#title' => t('Entity type'),
      '#options' => $entity_options,
    );

    $form['search']['entity_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Drupal Entity Id'),
      '#default_value' => '',
      '#size' => 19,
    );

    $form['search']['salesforce_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Salesforce Id'),
      '#default_value' => '',
      '#size' => 19,
      '#maxlength' => 18,
    );

    $form['search']['search_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Search'),
    );

    $form['bulk_operations'] = array(
      '#type' => 'fieldset',
      '#title' => t('Bulk Operations'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['bulk_operations']['operations'] = array(
      '#type' => 'select',
      '#options' => array(
        0 => t('Select a bulk operation'),
        'delete' => t('Delete selected mappings'),
      ),
    );

    $form['bulk_operations']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );

    $form['entities'] = array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $options,
      '#attributes' => array('class' => array('entity-sort-table')),
      '#empty' => t('There are no mappings.'),
    );

    return $form;
  }

  /**
   * Form Submit method.
   */
  public function overviewFormSubmit($form, &$form_state) {
    $values = $form_state['input'];
    $salesforce_mapping_object_ids = array();

    if (!empty($values['entities'])) {
      foreach ($values['entities'] as $index => $value) {
        if (!empty($value)) {
          $salesforce_mapping_object_ids[] = str_replace('salesforce_mapping_object_id-', '', $value);
        }
      }

      switch ($values['operations']) {
        case 'delete':
          drupal_goto('admin/content/salesforce_mapping_object/bulk/delete/' . implode('|', $salesforce_mapping_object_ids));
          break;
      }
    }

    if (!empty($values['search_text'])) {
      drupal_goto('admin/content/salesforce_mapping_object', array('query' => array('search' => $values['search_text'])));
    }
  }

}
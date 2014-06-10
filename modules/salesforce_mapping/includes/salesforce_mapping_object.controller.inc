<?php

/**
 * @file
 * Contains SalesforceMappingObjectController.
 */

/**
 * Salesforce Mapping UI controller.
 */
class SalesforceMappingObjectController extends EntityAPIController {

  /**
  * Override buildContent() to add properties.
  */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {

    $build = parent::buildContent($entity, $view_mode, $langcode);

    $wrapper = entity_metadata_wrapper('salesforce_mapping_object', $entity);

    foreach ($wrapper as $name => $property) {
      $info = $property->info();

      $content = salesforce_mapping_object_render_property($wrapper, $name, $property);

      if ($content) {
        $build[$name] = array(
          '#type' => 'item',
          '#title' => $info['label'] . ':',
          '#markup' => $content,
        );
      }
      elseif ($content !== FALSE) {
        unset($build[$name]);
      }
    }

    // Get historical/revision info:
    module_load_include('inc', 'salesforce_mapping', 'includes/salesforce_mapping_object.admin');

    $header = array(
      'salesforce_id' => array(
        'data' => t('Salesforce object'),
        'field' => 'salesforce_id',
      ),
      'last_sync_action' => array(
        'data' => t('Action'),
        'field' => 'last_sync_action',
      ),
      'last_sync' => array(
        'data' => t('Date/time'),
        'field' => 'last_sync',
        'sort' => 'desc',
      ),
      'last_sync_status' => array(
        'data' => t('Sync status'),
        'field' => 'last_sync_status',
      ),
      'last_sync_message' => array(
        'data' => t('Message'),
        'field' => 'last_sync_message',
      ),
    );

    $build['history'] = salesforce_mapping_object_overview_table($header, $entity->salesforce_mapping_object_id);

    return $build;
  }

}

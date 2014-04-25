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
   * Override parent to add columns.
   */
  protected function overviewTableHeaders($conditions, $rows, $additional_header = array()) {
    $additional_header[] = t('Drupal entity');
    $additional_header[] = t('Salesforce object');
    $additional_header[] = t('Last sync');
    return parent::overviewTableHeaders($conditions, $rows, $additional_header);
  }

  /**
   * Override parent to set the additional column values.
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    // Drupal entity.
    $drupal_entity = entity_load_single($entity->entity_type, $entity->entity_id);
    $uri = entity_uri($entity->entity_type, $drupal_entity);
    $label = entity_label($entity->entity_type, $drupal_entity);
    $additional_cols[] = l($label, $uri['path'], $uri['options']);

    $sfapi = salesforce_get_api();
    if ($sfapi) {
      $sf_id = l(
        $entity->salesforce_id,
        $sfapi->getInstanceUrl() . '/' . $entity->salesforce_id,
        array(
          'attributes' => array('target' => '_blank'),
        )
      );
    }
    else {
      $sf_id = $entity->salesforce_id;
    }
    $additional_cols[] = $sf_id;

    $additional_cols[] = format_date($entity->last_sync);
    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }

}
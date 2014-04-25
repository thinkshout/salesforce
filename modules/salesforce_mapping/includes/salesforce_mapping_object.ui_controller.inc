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
    // @TODO link to entity
    $drupal_entity = entity_load_single($entity->entity_type, $entity->entity_id);
    $additional_cols[] = entity_label($entity->entity_type, $drupal_entity);
    // @TODO link to SF object
    $additional_cols[] = $entity->salesforce_id;
    // @TODO convert to readable time
    $additional_cols[] = $entity->last_sync;
    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }

}
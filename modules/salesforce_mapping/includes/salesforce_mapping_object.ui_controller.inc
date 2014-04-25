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

}
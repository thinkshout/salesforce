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
    $items[$this->path]['title'] = t('Mapped objects');
    $items[$this->path]['description'] = t('Manage mapped Salesforce objects.');
    $items[$this->path]['access callback'] = 'salesforce_mapping_object_access';
    $items[$this->path]['access arguments'] = array('view salesforce mapping');
//    $items[$this->path]['type'] = MENU_LOCAL_TASK;
    return $items;
  }

}
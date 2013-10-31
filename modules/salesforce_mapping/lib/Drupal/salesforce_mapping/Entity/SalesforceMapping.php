<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Entity\SalesforceMapping.
 */

namespace Drupal\salesforce_mapping\Entity;

use Drupal\Core\Config\Entity\ConfigEntityBase;

/**
 * Defines a Salesforce Mapping configuration entity class.
 *
 * @EntityType(
 *   id = "salesforce_mapping",
 *   label = @Translation("Salesforce Mapping"),
 *   module = "salesforce_mapping",
 *   controllers = {
 *     "storage" = "Drupal\Core\Config\Entity\ConfigStorageController",
 *     "view_builder" = "Drupal\Core\Config\Entity\EntityViewBuilder",
 *     "access" = "Drupal\Core\Entity\EntityAccessController",
 *     "list" = "Drupal\Core\Config\Entity\ConfigEntityListController",
 *     "form" = {
 *       "edit" = "Drupal\salesforce_mapping\Form\SalesforceMappingFormController",
 *       "add" = "Drupal\salesforce_mapping\Form\SalesforceMappingFormController",
 *       "default" = "Drupal\salesforce_mapping\Form\SalesforceMappingFormController",
 *       "delete" = "Drupal\salesforce_mapping\Form\SalesforceMappingFormController"
 *      }
 *   },
 *   admin_permission = "administer salesforce mapping",
 *   config_prefix = "salesforce.salesforce_mapping",
 *   entity_keys = {
 *     "id" = "id",
 *     "label" = "label",
 *     "uuid" = "uuid",
 *     "status" = "status"
 *   },
 *   links = {
 *     "edit-form" = "admin/structure/salesforce/mappings/manage/{salesforce_mapping}"
 *   }
 * )
 */
class SalesforceMapping extends ConfigEntityBase {

  // Only one bundle type for now.
  public $type = 'salesforce_mapping';

  /**
   * Constructor for SalesforceMapping.
   *
   * @param array $values
   *   Associated array of values for the fields the entity should start with.
   */
  public function __construct(array $values = array()) {
    parent::__construct($values, 'salesforce_mapping');
  }

  /**
   * Save the entity.
   *
   * @return object
   *   The newly saved version of the entity.
   */
  public function save() {
    $this->updated = REQUEST_TIME;
    if (isset($this->is_new) && $this->is_new) {
      $this->created = REQUEST_TIME;
    }
    return parent::save();
  }

  // /**
  //  * Retreive the default URI.
  //  *
  //  * @return array
  //  *   Associated array with the default URI on the 'path' key.
  //  */
  // protected function defaultUri() {
  //   return array('path' => 'admin/structure/salesforce/mappings/manage/' . $this->identifier());
  // }

}

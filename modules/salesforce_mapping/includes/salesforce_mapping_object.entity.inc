<?php

/**
 * @file
 * Contains SalesforceMappingObject.
 */

/**
 * Entity class for Salesforce Mapping Objects
 */
class SalesforceMappingObject extends Entity {

  /**
   * Constructor for SalesforceMappingObject.
   *
   * @param array $values
   *   Associated array of values this entity should start with.
   */
  public function __construct(array $values = array()) {
    parent::__construct($values, 'salesforce_mapping_object');
  }

  /**
   * Save the entity.
   *
   * @return object
   *   The newly saved version of the entity.
   */
  public function save() {
    if (isset($this->is_new) && $this->is_new) {
      $this->created = REQUEST_TIME;
    }
    // @TODO make sense to autoset?
//    if (!isset($this->entity_updated) || !$this->entity_updated) {
//      $this->entity_updated = REQUEST_TIME;
//    }
    // @TODO make sense to autoset?
    if (!isset($this->last_sync) || !$this->last_sync) {
      $this->last_sync = REQUEST_TIME;
    }

    if (!isset($this->is_new_revision)) {
      $this->is_new_revision = TRUE;
    }

    if (!isset($this->default_revision)) {
      $this->default_revision = TRUE;
    }

    return parent::save();
  }

}

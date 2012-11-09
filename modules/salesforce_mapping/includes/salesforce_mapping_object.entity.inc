<?php

/**
 * Entity class for Salesforce Mapping Objects
 */
class SalesforceMappingObject extends Entity {

  public
    $salesforce_id,
    $entity_type,
    $entity_id,
    $created,
    $entity_updated,
    $last_sync;

  public function __construct(array $values = array()) {
    parent::__construct($values, 'salesforce_mapping_object');
  }

  public function save() {
    if (isset($this->is_new) && $this->is_new) {
      $this->created = REQUEST_TIME;
    }
    if (!$this->entity_updated) {
      $this->entity_updated = REQUEST_TIME;
    }
    if (!$this->last_sync) {
      $this->last_sync = REQUEST_TIME;
    }
    return parent::save();
  }

}

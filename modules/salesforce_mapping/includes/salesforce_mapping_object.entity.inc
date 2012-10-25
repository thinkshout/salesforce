<?php

/**
 * Entity class for Salesforce Mapping Objects
 */
class SalesforceMappingObject extends Entity {

  public
    $salesforce_id,
    $entity_type,
    $entity_id,
    $last_sync,
    $created;

  public function __construct(array $values = array()) {
    parent::__construct($values, 'salesforce_mapping_object');
  }

  public function save() {
    $this->last_sync = REQUEST_TIME;
    if (isset($this->is_new) && $this->is_new) {
      $this->created = REQUEST_TIME;
    }
    return parent::save();
  }

}

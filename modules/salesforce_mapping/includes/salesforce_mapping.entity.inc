<?php

/**
 * Entity class for Salesforce Mappings
 */
class SalesforceMapping extends Entity {

  public
    $salesforce_mapping_id,
    $name,
    $label,
    $type = 'salesforce_mapping', // only one bundle type for now
    $salesforce_object_type,
    $salesforce_record_type,
    $drupal_entity_type,
    $drupal_bundle,
    $field_mappings,
    $sync_triggers,
    $created,
    $updated,
    $weight,
    $push_async,
    $push_batch;

  public function __construct(array $values = array()) {
    parent::__construct($values, 'salesforce_mapping');
  }

  public function save() {
    $this->updated = REQUEST_TIME;
    if (isset($this->is_new) && $this->is_new) {
      $this->created = REQUEST_TIME;
    }
    return parent::save();
  }

  protected function defaultUri() {
    return array('path' => 'admin/structure/salesforce/mappings/manage/' . $this->identifier());
  }

}

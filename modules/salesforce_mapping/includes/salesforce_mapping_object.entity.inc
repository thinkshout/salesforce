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
   * Retrieve the default URI.
   *
   * @return array
   *   Associated array with the default URI on the 'path' key.
   */
  protected function defaultUri() {
    return array('path' => 'admin/content/salesforce/mapped-objects/' . $this->identifier());
  }

  /**
   * Retrieve the default label.
   */
  protected function defaultLabel() {
    $label = NULL;
    $wrapper = entity_metadata_wrapper('salesforce_mapping_object', $this);
    $label = $wrapper->entity->label();
    $label = ($label) ? $label : $this->entity_type . ':' . $this->entity_id;
    return t('@label to @sf_id', array('@label' => $label, '@sf_id' => $this->salesforce_id));
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
//    // @TODO make sense to autoset?
//    if (!isset($this->last_sync) || !$this->last_sync) {
//      $this->last_sync = REQUEST_TIME;
//    }

    if (!isset($this->is_new_revision)) {
      $this->is_new_revision = TRUE;
    }

    if (!isset($this->default_revision)) {
      $this->default_revision = TRUE;
    }

    return parent::save();
  }

}

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
   * Override buildContent() to add properties.
   */
  public function buildContent($view_mode = 'full', $langcode = NULL) {
    $content = array();
    $content = entity_get_controller($this->entityType)
      ->buildContent($this, $view_mode, $langcode, $content);

    $wrapper = entity_metadata_wrapper('salesforce_mapping_object', $this);

    $salesforce_url = $wrapper->salesforce_url->value();
    if ($salesforce_url) {
      $sf_object_link = l(
        $this->salesforce_id,
        $salesforce_url,
        array(
          'attributes' => array('target' => '_blank'),
        )
      );
    }
    else {
      $sf_object_link = $this->salesforce_id;
    }
    $content['salesforce_id']['content'] = $sf_object_link;

    return $content;
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

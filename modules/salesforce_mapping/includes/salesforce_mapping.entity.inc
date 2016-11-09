<?php

/**
 * @file
 * Contains SalesforceMapping.
 */

/**
 * Entity class for Salesforce Mappings
 */
class SalesforceMapping extends Entity {

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
    if (isset($this->is_new) && $this->is_new) {
      $this->created = REQUEST_TIME;
    }
    return parent::save();
  }

  /**
   * Return an array of Salesforce Field Names included in this mapping. 
   *
   * @param array $directions
   *   If given, only include fields mapped in this or these directions.
   *   Possible values are: 
   *     SALESFORCE_MAPPING_DIRECTION_SYNC
   *     SALESFORCE_MAPPING_DIRECTION_SF_DRUPAL 
   *     SALESFORCE_MAPPING_DIRECTION_DRUPAL_SF
   *   @see salesforce_mapping.module
   * @return array $mapped_fields
   *   Indexes and keys are both Salesforce Field (machine) Name
   */
  public function getMappedFields(array $directions = NULL) {
    $mapped_fields = array();
    foreach ($this->field_mappings as $field_map) {
      if (empty($directions) || in_array($field_map['direction'], $directions)) {
        // Some field map types (Relation) store a collection of SF objects.
        if (is_array($field_map['salesforce_field']) && !isset($field_map['salesforce_field']['name'])) {
          foreach ($field_map['salesforce_field'] as $sf_field) {
            $mapped_fields[$sf_field['name']] = $sf_field['name'];
          }
        }
        // The rest of are just a name/value pair.
        else {
          $mapped_fields[$field_map['salesforce_field']['name']] = $field_map['salesforce_field']['name'];
        }
      }
    }

    if (!empty($this->getMappedRecordTypes())) {
      $mapped_fields['RecordTypeId'] = 'RecordTypeId';
    }

    return $mapped_fields;
  }

  public function getMappedRecordTypes() {
    return $this->salesforce_record_type_default == SALESFORCE_MAPPING_DEFAULT_RECORD_TYPE 
      ? array()
      : array_filter($this->salesforce_record_types_allowed);
  }

  /**
   * Retreive the default URI.
   *
   * @return array
   *   Associated array with the default URI on the 'path' key.
   */
  protected function defaultUri() {
    return array('path' => 'admin/structure/salesforce/mappings/manage/' . $this->identifier());
  }

}

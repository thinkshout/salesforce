<?php

/**
 * Entity class for Salesforce Mappings
 */
class SalesforceMapping extends Entity {

  public
    $salesforce_mapping_id,
    $name,
    $salesforce_object_type,
    $salesforce_record_type,
    $drupal_entity_type,
    $drupal_bundle,
    $description,
    $field_mapping,
    $created,
    $updated;

}

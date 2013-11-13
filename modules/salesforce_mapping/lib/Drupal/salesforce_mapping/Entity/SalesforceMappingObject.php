<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Entity\SalesforceMappingObject.
 */

namespace Drupal\salesforce_mapping\Entity;

use Drupal\Core\Config\Entity\ContentEntityBase;

/**
 * Defines a Salesforce Mapping Object entity class. Mapping Objects are content
 * entities, since they're defined by references to other content entities.
 *
 * @EntityType(
 *   id = "salesforce_mapping_object",
 *   label = @Translation("Salesforce Mapping Object"),
 *   module = "salesforce_mapping",
 *   controllers = {
 *     "storage" = "Drupal\Core\Entity\DatabaseStorageController",
 *     "view_builder" = "Drupal\Core\Entity\EntityViewBuilder",
 *     "access" = "Drupal\Core\Entity\EntityAccessController",
 *   },
 *   admin_permission = "administer salesforce mapping",
 *   entity_keys = {
 *      "id" = "id",
 *      "entity_id" = "entity_id",
 *      "entity_type" = "entity_type",
 *      "salesforce_id" = "salesforce_id"
 *   }
 * )
 */
class SalesforceMappingObject extends ContentEntityBase {

  /**
   * {@inheritdoc}
   */
  public static function baseFieldDefinitions($entity_type) {
    $fields = array();
    $fields['id'] = array(
      'label' => t('Salesforce Mapping Object ID'),
      'description' => t('Primary Key: Unique salesforce_mapping_object entity ID.'),
      'type' => 'integer_field',
      'read-only' => TRUE,
    );
    $fields['entity_id'] = array(
      'label' => t('Entity ID'),
      'description' => t('Reference to the mapped Drupal entity.'),
      'type' => 'entity_reference_field',
      'required' => TRUE,
    );
    $fields['entity_type'] = array(
      'label' => t("Entity type"),
      'description' => t('Entity type for the mapped object.'),
      'type' => 'string_field',
      'required' => TRUE,
    );
    $fields['salesforce_id'] = array(
      'label' => t("Salesforce object identifier"),
      'description' => t('Reference to the mapped Salesforce object (SObject)'),
      'type' => 'salesforce_reference',
      'required' => TRUE,
    );
    $fields['created'] = array(
      'label' => t('Created'),
      'description' => t('The Unix timestamp when the object mapping was created.'),
      'type' => 'integer_field'
    );
    $fields['entity_updated'] = array(
      'label' => t('Drupal Entity Updated'),
      'description' => t('The Unix timestamp when the Drupal entity was last updated.'),
      'type' => 'integer_field'
    );
    $fields['last_sync'] = array(
      'label' => t('Last Sync'),
      'description' => t('The Unix timestamp when the record was last synched with Salesforce.'),
      'type' => 'integer_field'
    );
    return $fields;
  }

}

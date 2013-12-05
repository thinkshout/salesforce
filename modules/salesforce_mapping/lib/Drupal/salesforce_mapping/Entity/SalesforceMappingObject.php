<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Entity\SalesforceMappingObject.
 */

namespace Drupal\salesforce_mapping\Entity;

use Drupal\Core\Entity\ContentEntityBase;
use Drupal\salesforce_mapping\Entity\SalesforceMappingObjectInterface;

/**
 * Defines a Salesforce Mapping Object entity class. Mapping Objects are content
 * entities, since they're defined by references to other content entities.
 *
 * @EntityType(
 *   id = "salesforce_mapping_object",
 *   label = @Translation("Salesforce Mapping Object"),
 *   module = "salesforce_mapping",
 *   controllers = {
 *     "storage" = "Drupal\Core\Entity\FieldableDatabaseStorageController",
 *     "view_builder" = "Drupal\Core\Entity\EntityViewBuilder",
 *     "access" = "Drupal\Core\Entity\EntityAccessController",
 *   },
 *   base_table = "salesforce_mapping_object",
 *   admin_permission = "administer salesforce mapping",
 *   entity_keys = {
 *      "id" = "id",
 *      "entity_id" = "entity_id",
 *      "entity_type" = "entity_type",
 *      "salesforce_id" = "salesforce_id"
 *   }
 * )
 */
class SalesforceMappingObject extends ContentEntityBase implements SalesforceMappingObjectInterface {

  /**
   * Overrides ContentEntityBase::__construct().
   */
  public function __construct(array $values) {
    parent::__construct($values, 'salesforce_mapping_object');
}

  public function save() {
    if ($this->isNew()) {
      $this->created = REQUEST_TIME;
    }

    // The caller should decide whether the entity updated or synched
    // if (!$this->entity_updated) {
    //   $this->entity_updated = REQUEST_TIME;
    // }
    // if (!$this->last_sync) {
    //   $this->last_sync = REQUEST_TIME;
    // }
    return parent::save();
  }

  /**
   * {@inheritdoc}
   */
  public static function baseFieldDefinitions($entity_type) {
    // @todo Do we really have to define this, and hook_schema, and entity_keys?
    // so much redundancy.
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
      'type' => 'integer_field',
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
      'type' => 'string_field',
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

  public function getSalesforceLink($options = array()) {
    $defaults = array('attributes' => array('target' => '_blank'));
    $options = array_merge($defaults, $options);
    return l($this->sfid(), $this->getSalesforceUrl(), $options);
  }

  public function getSalesforceUrl() {
    // @todo dependency injection here:
    $sfapi = salesforce_get_api();
    if (!$sfapi) {
      return $this->salesforce_id->value;
    }
    return $sfapi->getInstanceUrl() . '/' . $this->salesforce_id->value;
  }

  public function sfid() {
    return $this->get('salesforce_id')->value;
  }
}

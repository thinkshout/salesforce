<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Entity\SalesforceMapping.
 */

namespace Drupal\salesforce_mapping\Entity;

use Drupal\Core\Config\Entity\ConfigEntityBase;
use Drupal\salesforce_mapping\Plugin\FieldPluginManager;
use Drupal\Core\Entity\EntityInterface;

/**
 * Defines a Salesforce Mapping configuration entity class.
 *
 * @EntityType(
 *   id = "salesforce_mapping",
 *   label = @Translation("Salesforce Mapping"),
 *   module = "salesforce_mapping",
 *   controllers = {
 *     "storage" = "Drupal\Core\Config\Entity\ConfigStorageController",
 *     "view_builder" = "Drupal\Core\Entity\EntityViewBuilder",
 *     "access" = "Drupal\salesforce_mapping\SalesforceMappingAccessController",
 *     "list" = "Drupal\salesforce_mapping\SalesforceMappingList",
 *     "form" = {
 *       "edit" = "Drupal\salesforce_mapping\Form\SalesforceMappingEditForm",
 *       "add" = "Drupal\salesforce_mapping\Form\SalesforceMappingEditForm",
 *       "disable" = "Drupal\salesforce_mapping\Form\SalesforceMappingDisableForm",
 *       "delete" = "Drupal\salesforce_mapping\Form\SalesforceMappingDeleteForm",
 *       "enable" = "Drupal\salesforce_mapping\Form\SalesforceMappingEnableForm",
 *       "fields" = "Drupal\salesforce_mapping\Form\SalesforceMappingFieldsForm",
 *      }
 *   },
 *   admin_permission = "administer salesforce mapping",
 *   config_prefix = "salesforce.salesforce_mapping",
 *   entity_keys = {
 *     "id" = "id",
 *     "label" = "label",
 *     "uuid" = "uuid",
 *     "status" = "status",
 *     "weight" = "weight",
 *   },
 *   links = {
 *     "edit-form" = "admin/structure/salesforce/mappings/manage/{salesforce_mapping}"
 *   }
 * )
 */
class SalesforceMapping extends ConfigEntityBase {

  // Only one bundle type for now.
  public $type = 'salesforce_mapping';

  // @todo a little overboard on the properties. Can probably ditch these and force callers to use ->get or ->config

  /**
   * ID (machine name) of the Mapping
   * @note numeric id was removed
   *
   * @var string
   */
  public $id;

  /**
   * Label of the Mapping
   *
   * @var string
   */
  public $label;

  /**
   * The UUID for this entity.
   *
   * @var string
   */
  public $uuid;

  /**
   * A default weight for the mapping.
   *
   * @var int (optional)
   */
  public $weight = 0;

  /**
   * Status flag for the mapping.
   *
   * @var boolean
   */
  public $status = TRUE;

  
  /**
   * The drupal entity type to which this mapping points
   *
   * @var string
   */
  public $drupal_entity_type;

  /**
   * The drupal entity bundle to which this mapping points
   *
   * @var string
   */
  public $drupal_bundle;

  /**
   * The salesforce object type to which this mapping points
   *
   * @var string
   */
  public $salesforce_object_type;

  /**
   * The salesforce record type to which this mapping points, if applicable
   *
   * @var string (optional)
   */
  public $salesforce_record_type = '';

  /**
   * ID of field plugin marked as "key" (in field_mappings)
   */
  private $key_field_id = NULL;

  /**
   * cache copy of key field plugin object from field_mappings
   */
  private $key_field_plugin;

  /** 
   * quick bool to know whether or not we should even check for key value.
   */
  private $has_key = NULL;

  /**
   * @todo documentation
   */
  public $field_mappings = array();
  public $sync_triggers = array();
  public $push_async;
  public $push_batch;

  protected $fieldManager;

  /**
   * {@inheritdoc}
   */
  public function __construct(array $values = array(), $entity_type) {
    parent::__construct($values, $entity_type);
    // entities don't support Dependency Injection, so we have to build a hard
    // dependency on the container here.
    $this->fieldManager = \Drupal::service('plugin.manager.salesforce_mapping.field');

    // Initialize our private key field tracker for easy access.
    foreach ($this->field_mappings as $key => $value) {
      if ($value['key']) {
        $this->key_field_id = $key;
      }
    }

    if (empty($this->field_mappings[$this->key_field_id])) {
      $this->has_key = FALSE;
      return;
    }
    $fieldmap = $this->field_mappings[$this->key_field_id];
    $this->key_field_plugin = $this->fieldManager->createInstance($fieldmap['drupal_field_type'], $fieldmap);
    // Just double-check that we have something there:
    if ($this->key_field_plugin->config('salesforce_field')) {
      $this->has_key = TRUE;
    }
  }

  /**
   * Save the entity.
   *
   * @return object
   *   The newly saved version of the entity.
   */
  public function save() {
    $this->updated = REQUEST_TIME;
    if (isset($this->is_new) && $this->is_new) {
      $this->created = REQUEST_TIME;
    }
    foreach ($this->field_mappings as $i => $fieldmap) {
      if ($fieldmap['key']) {
        $this->key_field_id = $i;
        break;
      }
    }
    return parent::save();
  }

  /**
   * Given a Drupal entity, return an array of Salesforce key-value pairs 
   *
   * @param object $entity
   *   Entity wrapper object.
   *
   * @return array
   *   Associative array of key value pairs.
   * @see salesforce_push_map_params (from d7)
   */
  public function getPushParams(EntityInterface $entity) {
    // @todo This should probably be delegated to a field plugin bag?
    foreach ($this->field_mappings as $fieldmap) {
      $field_plugin = $this->fieldManager->createInstance($fieldmap['drupal_field_type'], $fieldmap);
      // Skip fields that aren't being pushed to Salesforce.
      if (!$field_plugin->push()) {
        continue;
      }
      $params[$field_plugin->config('salesforce_field')] = $field_plugin->value($entity);
    }
    // @todo make this an event
    drupal_alter('salesforce_push_params', $params, $mapping, $entity_wrapper);
    return $params;
  }

  /**
   * Get the name of the salesforce key field, or NULL if no key is set.
   */
  public function getKeyField() {
    if (!$this->hasKey()) {
      return;
    }
    return $this->key_field_plugin->config('salesforce_field');
  }

  public function hasKey() {
    return $this->has_key;
  }

  public function getKeyValue(EntityInterface $entity) {
    if (!$this->hasKey()) {
      return;
    }
    return $ths->key_field_plugin->config('salesforce_field');
  }

  public function pull() {
    
  }

}

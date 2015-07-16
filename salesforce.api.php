<?php

/**
 * @file
 * These are the hooks that are invoked by the Salesforce core.
 *
 * Core hooks are typically called in all modules at once using
 * module_invoke_all().
 */

/**
 * @defgroup salesforce_hooks Hooks provided by Salesforce API
 * @{
 */

/**
 * Trigger action when first building the list of fieldmap types.
 */
function hook_salesforce_mapping_fieldmap_type() {

}

/**
 * Alter existing fieldmap types.
 *
 * @param array $fieldmap_type
 *   Array of fieldmap Salesforce types
 */
function hook_salesforce_mapping_fieldmap_type_alter($fieldmap_type) {

}

/**
 * Alter parameters mapped to a Salesforce object before syncing to Salesforce.
 *
 * @param array $params
 *   Associative array of key value pairs.
 * @param object $mapping
 *   Salesforce mapping object.
 * @param object $entity_wrapper
 *   EntityMetadataWrapper of entity being mapped.
 */
function hook_salesforce_push_params_alter(&$params, $mapping, $entity_wrapper) {

}

/**
 * Prevent push to SF for an entity.
 *
 * @param string $entity_type
 *   The type of entity the push is for.
 * @param object $entity
 *   The entity object the push is for.
 * @param ing $sf_sync_trigger
 *   Constant for the Drupal operation that triggered the sync.
 *
 * @return bool
 *   FALSE if the entity should not be synced to Salesforce for the
 *   $sf_sync_trigger operation.
 */
function hook_salesforce_push_entity_allowed($entity_type, $entity, $sf_sync_trigger) {

}

/**
 * Alter the value being mapped to an entity property from a Salesforce object.
 *
 * @param string $value
 *   Salesforce field value.
 * @param array $field_map
 *   Associative array containing the field mapping in the form
 *   <code>
 *   'fieldmap_name' => array(
 *      'drupal_field' => array(
 *        'fieldmap_type' => 'property',
 *        'fieldmap_value' => 'first_name'
 *      ),
 *      'salesforce_field' => array()
 *   )
 *   </code>
 * @param object $sf_object
 *   Fully loaded Salesforce object
 */
function hook_salesforce_pull_entity_value_alter(&$value, $field_map, $sf_object) {

}

/**
 * Alter a SOQL select query before it is executed.
 *
 * @param SalesforceSelectQuery $query
 *   The query object to alter.
 */
function hook_salesforce_query_alter(SalesforceSelectQuery &$query) {

}

/**
 * A salesforce push has failed: Implementations may wish to react, for
 * example, by logging the failure or alerting an administrator.
 *
 * @param string $op
 *   The salesforce operation: Create, Update, Upsert, or Delete
 * @param object $result
 *   The salesforce response
 * @param array $synced_entity
 *   Entity data for this push. This array has 3 keys
 *     'entity_wrapper': entity_metadata_wrapper() for the Drupal entity 
 *     'mapping_object': salesforce mapping object record, if it exists. 
 *       Otherwise null
 *     'queue_item': Drupal queue item corresponding to this push attempt
 */
function hook_salesforce_push_fail($op, $result, $synced_entity) {
  
}
$synced_entities[$delta] = array(
  'entity_wrapper' => $wrapper,
  'mapping_object' => $mapping_object,
  'queue_item' => $item,
);
  

/**
 * @} salesforce_hooks
 */

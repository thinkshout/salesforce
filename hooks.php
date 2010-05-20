<?php
// $Id$

/**
 * @file
 * These are the hooks that are invoked by the Salesforce core.
 *
 * Core hooks are typically called in all modules at once using
 * module_invoke_all().
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Expose fields to fieldmappings.
 *
 * Salesforce_api does not expose any Drupal fields. It's up to modules
 * (e.g. sf_user and sf_node) to make those fields available for mapping.
 * Developers implementing this hook should pay close attention to the
 * import/export functions for each field, which are responsible for delivering
 * the actual data for mapped fields. If import or export indexes are unset,
 * salesforce_api will use the property of the object with the same fieldname.
 * For example, "nid" does not have an import/export function, because "nid" is
 * a property of the $node object.
 *
 * @param $object_type
 *  Where does the data come from? Either "drupal" or "salesforce".
 * @return
 *  The function should return an associative array of entities, each   
 *  describing its bundles and the fields that should be made available for
 *  mapping. Each field is an associative array with the following keys
 *  (optional unless noted):
 *  - 'label' (required): The translated, user-friendly name of this field
 *  - 'type': Relevant for Salesforce fields only.
 *    Use one of the following constants
 *    - SALESFORCE_FIELD_OPTIONAL (default): An optional field
 *    - SALESFORCE_FIELD_REQUIRED: A required field (e.g. username, LastName)
 *    - SALESFORCE_FIELD_SOURCE_ONLY: An automatically assigned field (e.g. nid, CreatedDate)
 *  - 'import': callback function to import this field.
 *  - 'export': callback function to export this field.
 *  - 'multiple': Is this a multiple-valued field in Salesforce? (ie. ;-delimited)
 *  - 'group': Assign the field to a grouping on the field mapping UI
 */
function hook_fieldmap_objects($object_type) {
  if ($type == 'drupal') {
    return array(
      // "node" is the entity
      'node' => array(
        // "page" is the bundle
        'page' => array(
          'label' => t('Page node'),
          'fields' => array(
            'nid' => array('label' => t('Node ID'), 'type' => SALESFORCE_FIELD_SOURCE_ONLY),
            'type' => array('label' => t('Node type')),
            'status' => array('label' => t('Is the node published?')),
            'field_sample_checkbox' => array(
              'label' => t('Widget Label'),
              'group' => t('CCK fields'),
              'export' => '_sf_node_export_cck_default',
              'import' => '_sf_node_import_cck_default',
              'multiple' => TRUE,
    )))));
  }
}


/**
 * Modify fieldmap object definitions.
 *
 * @param $objects
 *  The fieldmap object definition as defined by hook_fieldmap_objects implementations.
 */
function hook_fieldmap_objects_alter(&$objects) {
  $objects['node']['page']['fields']['status']['label'] = "Published Status";
}

/**
 * Retrieve matching object ids before creating a new object. This hook is 
 * designed to eliminate creation of duplicate objects if so desired. For 
 * example, an implementation of sf_user_sf_find_match might query Salesforce
 * for Ids matching the user's email address before creating a new Contact.
 *
 * @param $direction
 *  "import" or "export"
 * @param $fieldmap_type
 *  "user", "node", etc.
 * @param $object
 *  The data object, probably $user or $node
 * @param $fieldmap_id
 *  The id of the fieldmap being used to import or export the current object.
 * @return
 *  'import': an array of matching nid's, uid's, etc.
 *  'export': an array of matching Salesforce Id's
 */
function hook_sf_find_match($direction, $fieldmap_type, $object, $fieldmap_id) {
  if ($direction == 'export' 
    && ($fieldmap_type == 'user' || ($fieldmap_type == 'node' && $object == 'profile'))) {
    if (empty($object->mail)) {
      $object->mail = db_result(db_query('SELECT mail FROM {user} WHERE uid = %d', $object->uid));
    }
    $sf = salesforce_api_connect();
    if (!is_object($sf)) {
      watchdog('sf_find_match', 'SalesForce connection failed when looking for a match.');
      return;
    }
    $result = $sf->client->query('SELECT Id FROM Contact WHERE Email = \''.$obj->mail.'\'');
    if (count($result->records)) {
      return array($result->records[0]->Id);
    }
  }
}

/**
 * @} End of "addtogroup hooks".
 */

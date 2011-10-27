<?php

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
 * Modify Salesforce IDs before they are saved to the {salesforce_object_map}
 * table.
 * @param $drupal_type
 *   The type of the Drupal object you are requesting data for; node or user.
 * @param $oid
 *   The associated unique ID used to identify the object in Drupal.
 * @param $sfid
 *   The Salesforce ID of the associated object in the Salesforce database.
 * @param $name
 *   The name of the fieldmap used to generate the export.
 * @param string $op_type
 *   The operation being performed, 'import' or 'export'
 * @return
 *   TRUE if salesforce_api_id_save() should proceed with saving the link, FALSE
 *   otherwise
*/
function hook_salesforce_api_id_save_alter(&$drupal_type, &$oid, &$sfid, &$name, &$op_type) {
  // Example: Do not allow a mapping to be saved between UID 1 and Salesforce
  if ($oid == 1 && $drupal_type == 'user') {
    return FALSE;
  }
}
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
 * For CCK fields, something special happens. When building the list of objects
 * salesforce will attempt to locate export/import functions based on the naming  
 * convention _sf_node_export_cck_FIELDTYPE and _sf_node_import_cck_FIELDTYPE.
 * For example, see _sf_node_export_cck_date and _sf_node_import_cck_date in 
 * sf_contrib. The default CCK handler will expose all columns from all CCK 
 * field types for export and import to and from Salesforce. 
 * 
 * If the value of a particular column is not useful on its own, or if it needs 
 * to be manipulated in a specific way before being sent to Salesforce, then an
 * export (and/or import as appropriate) override should be declared according
 * to the naming convention (_sf_node_export_cck_FIELDTYPE). Any such function
 * will automatically be used to export/import ALL columns for the CCK field 
 * type. Simple CCK fields with only a "value" column will be named after their
 * field_name properties. CCK fields with columns other than "value" will be 
 * referred to according to the convention: FIELDNAME:COLUMN
 * @see _sf_node_export_cck_FIELDTYPE
 * @see _sf_node_import_cck_FIELDTYPE
 * 
 * If you want to expose additional non-cck fields for mapping, you should 
 * implement this hook, hook_fieldmap_objects.
 * 
 * If you want to change the default definition of a field or fields, 
 * @see hook_fieldmap_objects_alter
 *
 * @param $object_type
 *  Where does the data come from? Either "drupal" or "salesforce".
 * @return
 *  The function should return an associative array of objects and their
 *  fields that should be made available for mapping. Each field is an
 *  associative array with the following keys (optional unless noted):
 *  - 'label' (required): The translated, user-friendly name of this field
 *  - 'type': Relevant for Salesforce fields only.
 *    Use any combination of the following bitmasks, |'d (bitwise OR) together
 *    - SALESFORCE_FIELD_CREATEABLE
 *    - SALESFORCE_FIELD_DEFAULTEDONCREATE
 *    - SALESFORCE_FIELD_DEPRECATEDANDHIDDEN
 *    - SALESFORCE_FIELD_IDLOOKUP
 *    - SALESFORCE_FIELD_NILLABLE
 *    - SALESFORCE_FIELD_RESTRICTEDPICKLIST
 *    - SALESFORCE_FIELD_UNIQUE
 *    - SALESFORCE_FIELD_UPDATEABLE
 *    - SALESFORCE_FIELD_SOURCE_ONLY
 *  - 'import': callback function to import this field.
 *  - 'export': callback function to export this field.
 *  - 'multiple': Is this a multiple-valued field in Salesforce? (ie. ;-delimited)
 *  - 'group': Assign the field to a grouping on the field mapping UI
 */
function hook_fieldmap_objects($object_type) {
  if ($type == 'drupal') {
    return array(
      'node_page' => array(
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
    ))));
  }
}

/**
 * Modify fieldmap object definitions.
 *
 * @param $objects
 *  The fieldmap object definition as defined by hook_fieldmap_objects implementations.
 */
function hook_fieldmap_objects_alter(&$objects) {
  $objects['node_page']['fields']['status']['label'] = "Published Status";
}

/**
 * Retrieve matching object ids before creating a new object. This hook is 
 * designed to eliminate creation of duplicate objects if so desired. For 
 * example, an implementation of sf_user_sf_find_match might query Salesforce
 * for Ids matching the user's email address before creating a new Contact.
 *
 * IMPORTANT: implementations of this function MUST ensure that matches are not
 * IDs of deleted records in Salesforce. By default SOQL query() filters out
 * deleted records, so sf_prematch_sf_find_match() fulfils this requirement.
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
 *  'export': an array of matching Salesforce Id's @see IMPORTANT note above.
 */
function hook_sf_find_match($direction, $fieldmap_type, $object, $fieldmap_name) {
  if ($direction == 'export' 
    && ($fieldmap_type == 'user' || ($fieldmap_type == 'node' && $object == 'profile'))) {
    if (empty($object->mail)) {
      $object->mail = db_result(db_query('SELECT mail FROM {user} WHERE uid = %d', $object->uid));
    }
    $sf = salesforce_api_connect();
    if (!is_object($sf)) {
      watchdog('sf_find_match', 'Salesforce connection failed when looking for a match.');
      return;
    }
    $result = $sf->client->query('SELECT Id FROM Contact WHERE Email = \''.$obj->mail.'\'');
    if (count($result->records)) {
      return array($result->records[0]->Id);
    }
  }
}

/**
 * Builds a set of default field maps. This allows modules to offer out-of-the 
 * box mappings based on common use cases or patterns. It is recommended but not 
 * required that you use the following verbose naming convention for your
 * default fieldmaps in order to avoid namespace collisions:
 *   mymodule_default_drupalentity_sfentity_field_map
 * The name of your default fieldmap will serve as its primary identifier.
 * If/when your fieldmap is overridden, it will be assigned a standard fieldmap 
 * id which will then be used to identify the fieldmap. This is a bastardization
 * of the model ctools uses in order to avoid hard dependency on ctools.
 * 
 * Finally, it is highly recommended that your default fieldmap NOT be 
 * automatic. Remember that out-of-the-box module behavior should not change
 * Drupal's current working configuration.
 *
 * @param string $export - the export schema definition with defaults applied. 
 *   (generally unused)
 * @return an array of fieldmap objects according to salesforce_field_map schema
 * @see salesforce_api_default_salesforce_field_maps
 */

function hook_default_salesforce_field_maps($export = array()) {
  return array( (object) array(
    'disabled' => FALSE,
    'name' => 'salesforce_api_default_user_contact_field_map',
    'automatic' => FALSE,
    'drupal' => 'user',
    'salesforce' => 'Contact',
    'fields' => array('LastName' => 'name', 'Email' => 'mail'),
    'description' => 'This is a simple example fieldmap to get you started using the Salesforce API.',
    ));
}

/**
 * Called immediately before a Salesforce object is to be created or updated
 * during export (e.g. sf_user_export, sf_node_export). Entity-based modules
 * that invoke Salesforce create, update, or upsert methods should always invoke
 * this hook before calling Salesforce. This feature was primarily designed for
 * queue/batch support, but could have other use cases as well.
 * 
 * This hook is invoked after a prematching attempt. For modules that may care
 * about such a situation, when $sf_object->sfid is empty, this is a Salesforce 
 * "create" operation. Otherwise, this is a Salesforce "update" operation.
 *
 * @param object $sf_object
 *   The object about to be exported to Salesforce
 * @param mixed $name
 *   The fieldmap name used to create sf_object
 * @param string $drupal_id 
 *   The unique id of the drupal object associated with the sf_object, e.g. nid
 * @param array $map
 *   The fieldmap array provided by salesforce_api_fieldmap_load().
 * @return
 *   Implementing modules should return FALSE if the current export should NOT
 *   proceed. Note that this will prevent further processing of implementations
 *   of this hook.
 */
function hook_salesforce_api_pre_export(&$sf_object, $map, $drupal_id) {

}

/**
 * Called after a Salesforce create or update attempt.
 * @see hook_salesforce_api_pre_export
 *
 * @param string $sf_object
 * @param string $name
 * @param string $drupal_id 
 * @param object $salesforce_response
 *   The response object from the Salesforce SOAP server. This object has three
 *   properties:
 *   - errors: A single error array, or an an array of one or more errors with
 *     three keys:
 *     - fields: A single field name, or an array of one or more field names
 *     - message: A human readable failure message
 *     - statusCode: A machine readable failure code
 *   - id: If there was no error, the Salesforce id of the touched object.
 *   - success: boolean
 * @return void
 */
function hook_salesforce_api_post_export($sf_object, $name, $drupal_id, $salesforce_response) {
  
}

/**
 * Called immediately before creating or updating a Drupal object from
 * Salesforce data import
 *
 * @param object $object - the drupal object (e.g. node, account) about to be
 *  created or updated
 * @param string $name - the name of the fieldmap used for import
 * @param object $sf_data - the data received from Salesforce 
 * @param array $changes - if this is a user import, the $changes array as it 
 *  will be passed to user_save(). Otherwise NULL.
 * @return
 *   Implementing modules should return FALSE if the current import should NOT
 *   proceed. Note that this will prevent further processing of implementations
 *   of this hook.
 */
function hook_salesforce_api_pre_import(&$object, $name, $sf_data, &$changes = NULL) {
  
}

/**
 * Called immediately after creating or updating a Drupal object from Salesforce
 * data import.
 *
 * @param object $object - the drupal object (e.g. node, account) just
 *  created or updated
 * @param string $name - the name of the fieldmap used for import
 * @param object $sf_data - the data received from Salesforce
 * @param string $create
 * @param array $changes - if this is a user import, the $changes array as it
 *  was passed to user_save(). Otherwise null.
 * @return void
 */
function hook_salesforce_api_post_import($object, $name, $sf_data, $create, $changes = NULL) {
  
}

/**
 * Called when a connection attempt to Salesforce fails. 
 * @see salesforce_api_export()
 *
 * @param string $drupal_objects
 *   The Drupal objects that were being exported.
 * @param string $name
 *   The name of the fieldmap to have been used for the export.
 * @param string $sfid
 *   Any sfid or sfids to have been used for export, if provided.
 */
function hook_salesforce_api_export_connect_fail($drupal_objects, $name, $sfid) {
  // respond to connection failure
}

/**
 * Called before Salesforce delete
 * @see hook_salesforce_api_pre_export
 * This hook can be used to prevent deletion of Salesforce records entities, but
 * cannot prevent deletion of Drupal entities (@see hook_nodeapi or hook_user).
 *
 * @param object $sfid
 * @param object $map
 * @param string $drupal_id
 * @return FALSE if the Salesforce record should not be deleted.
 */
function hook_salesforce_api_delete($sfid, $map, $drupal_id) {
  
}

/**
 * Called after a link between Drupal and SF objects is removed. $args is the
 * array of ids which were provided to salesforce_api_id_unlink().
 * @see salesforce_api_id_unlink() for full explanation of arguments.
 */
function hook_salesforce_api_post_unlink($args) {
  
}


/**
 * Override the default CCK export callback for a cck field type.
 *
 * @param object $node
 * @param string $fieldname 
 * @param array $drupal_field_definition 
 * @param array $sf_field_definition 
 * @return the value for export to Salesforce
 */
function _sf_node_export_cck_FIELDTYPE($node, $fieldname, $drupal_field_definition, $sf_field_definition) {
  list($fieldname, $column) = explode(':', $fieldname, 2);
  if (empty($column)) {
    $column = 'value';
  }
  return _sf_node_export_date($node->$fieldname[0][$column]);
}

/**
 * Override the default CCK import callback for a cck field type. This function
 * should take $node by reference and make any necessary changes before the node
 * is saved.
 *
 * @param object $node
 * @param string $drupal_fieldname
 * @param array $drupal_field_definition
 * @param object $sf_data
 * @param string $sf_field_definition
 * @param array $sf_field_definition
 * @return null
 */
function _sf_node_import_cck_FIELDTYPE(&$node, $drupal_fieldname, $drupal_field_definition, $sf_data, $sf_fieldname, $sf_field_definition) {
  list($fieldname, $column) = explode(':', $fieldname, 2);
  if (empty($column)) {
    $column = 'value';
  }
  $node->$drupal_fieldname[0][$column] = strtotime($source->$sf_fieldname);  
}

/**
 * @} End of "addtogroup hooks".
 */

<?php

/**
 * @file
 * These are the hooks that are invoked by the Salesforce Notifications module.
 *
 * Hooks are typically called in all modules at once using
 * module_invoke_all().
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Indicate whether a notification should be processed.
 *
 * This hook is invoked prior to processing each incoming notification from
 * Salesforce. You may use this hook to set conditions under which a
 * notification will be processed. Notifications failing these conditions will
 * not trigger this fieldmap. This may be particularly useful in the case of
 * Salesforce objects that are mapped to multiple Drupal objects.
 *
 * @param $operation
 *  The operation being performed. Either "insert", "update", or "delete".
 * @param $object_record
 *  The object record we are performing the operation on. An array with the
 *  following keys:
 *  - oid: The drupal object id of the object we are operating on. NULL if
 *    $operation is 'insert'.
 *  - name: The name of the fieldmap to be used.
 *  - drupal_entity: The entity type this object should be inserted as.
 *  - drupal_bundle: The bundle this object should be inserted as.
 *  - fields: The fields provided in the incoming Salesforce notification.
 *  - operation: The operation we are performing. The same as the $operation
 *    parameter.
 * @param $map
 *  The fieldmap object containing the fieldmap we want to use.
 * @return
 *  Boolean TRUE or FALSE depending on whether this object should be processed
 *  or not.
 */
function hook_sf_notifications_check_condition($operation, $object_record, $map) {
  switch ($operation) {
    case "insert":
    case "update":
      return TRUE;
    default:
      return FALSE;
  }
}

/**
 * Hook called after a notification message has been processed.
 *
 * This hook is invoked after a notification has been processed. You can use
 * this hook to perform any additional actions required after a node has been
 * created, updated, or deleted as a result of a Salesforce notifications
 * message.
 *
 * @param $operation
 *  The operation being performed. Either "insert", "update", or "delete".
 * @param $object_record
 *  The object record the operation has been performed on. An array with the
 *  following keys:
 *  - oid: The Drupal object id of the object we are operating on. NULL if
 *    $operation is 'insert'.
 *  - name: The name of the fieldmap to be used.
 *  - drupal_entity: The entity type this object should be inserted as.
 *  - drupal_bundle: The bundle this object should be inserted as.
 *  - fields: The fields provided in the incoming Salesforce notification.
 *  - operation: The operation we are performing. The same as the $operation
 *    parameter.
 * @param $drupal_id
 *   The Drupal object id of the object just inserted or updated. This parameter
 *   will not be set on the case of deletion.
 */
function hook_sf_notifications_processed($operation, &$object_record) {
  switch ($operation) {
    case "insert":
    case "update":
      return TRUE;
    default:
      return FALSE;
  }
}

/**
 * @} End of "addtogroup hooks".
 */

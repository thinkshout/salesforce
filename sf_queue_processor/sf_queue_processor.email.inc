<?php

/**
 * @file
 * Creates email messages for the queue processor module.
 */

/**
 * Generates the object error email.
 */
function sf_object_error_email(&$object, &$errors) {

  $data = unserialize($object->data);

  $message = t("The following API errors were returned from Salesforce.\n\r");
  $message .= "---------------------------------------------------------\n\r\n\r";
  foreach ($errors as $error) {
    $message .= "Status Code: " . $error->statusCode . "\n\r";
    $message .= "Message: " . $error->message . "\n\r";
    $message .= "Fields: " . $error->fields . "\n\r\n\r";
  }

  $message .= format_object_details($data);
  $message .= "\n\r\n\r";
  $message .= 'To edit the object, go to ' . created_edit_link($object->oid, $object->type);
  return $message;
}

/**
 * Generates the retry queue email.
 */
function sf_moved_to_retry_queue_email(&$object) {

  $data = unserialize($object->data);
  $retry_details = get_retry_queue_details($object->oid, $object->type);

  $message = t("This item has been placed in the retry queue and will be exported again on !date. If you do not fix the error before then, the object will fail again. This object has !retries retries remaining.\n\r\n\r",
		array('!date' => date('l, F j Y', $retry_details['retry_date']), '!retries' => $retry_details['attempts_remaining']));

  $message .= format_object_details($data);
  $message .= "\n\r\n\r";
  $message .= 'To edit the object, go to ' . created_edit_link($object->oid, $object->type);
  return $message;
}

/**
 * Generates the permanent failure email.
 */
function sf_permanent_failure_email(&$object) {

  $data = unserialize($object->data);
  $message = t("This item has been marked as a permanent failure. It will be excluded from future exports until it has been edited.\n\r\n\r");

  $message .= format_object_details($data);
  $message .= "\n\r\n\r";
  $message .= 'To edit the object, go to ' . created_edit_link($object->oid, $object->type);
  return $message;
}

/**
 * Generates SOAP fault email.
 */
function sf_soap_fault_email() {

  $message = t("A fatal SOAP fault has occurred when attempting to connect to Salesforce.");
  return $message;
}

/**
 * Generates connection error email.
 */
function sf_connection_error_email() {

  global $base_url;
  $sf_settings = $base_url . '/admin/config/salesforce/settings';

  $message = t("A connection with Salesforce could not be established. Typical reasons include Salesforce being down for maintenance or incorrect credentials have been provided for login.\n\r\n\r");
  $message .= t("If Salesforce is not down for maintenance, please double check the credentials at !url.\n\r\n\r", array('!url' => $sf_settings));
  $message .= t("All items will remain in the export queue until the next cron run.\n\r\n\r");

  return $message;
}

/**
 * Creates a summary report of the cron run.
 */
function sf_queue_summary_email(&$batches) {
  $message = t("!num batches were processed during the run\n\r\n\r", array('!num' => count($batches)));

  // summary
  foreach ($batches as $batch) {
    $message .= "******************************************************\n\r";
    $message .= t("BATCH !id\n\r", array('!id' => $batch['id']));
    $message .= "******************************************************\n\r";
    $message .= t("Type: !type\n\r", array('!type' => $batch['type']));
    $message .= t("Field Map: !fieldmap\n\r", array('!fieldmap' => $batch['fieldmap']));
    $message .= t("Dedupe Key: !dedupe_key\n\r", array('!dedupe_key' => $batch['dedupe_key']));
    $message .= t("Number of items: !count\n\r", array('!count' => count($batch['records'])));
    $message .= t("Success count: !success_count\n\r", array('!success_count' => $batch['success_count']));
    $message .= t("Failure count: !failure_count\n\r", array('!failure_count' => $batch['failure_count']));

    // add failures (if any)
    if ($batch['failure_count'] > 0) {
      $message .= "\n\rFailures\n\r";
      $message .= "----------------------------------------------------\n\r";
      $errors = load_batch_errors($batch['id']);
      foreach ($errors as $error) {
        $message .= t("ID: !oid\n\r", array('!oid' => $error->oid));
        $message .= t("Status Code: !status_code\n\r", array('!status_code' => $error->status_code));
        $message .= t("Message: !message\n\r", array('!message' => $error->message));
        $message .= t("Fields: !fields\n\r", array('!fields' => $error->fields));
        $message .= t("To edit the object, go to !link\n\r\n\r", array('!link' => created_edit_link($error->oid, $error->type)));
      }
      $message .= "\n\r";
    }
    else {
      $message .= "\n\r\n\r";
    }
  }
  return $message;
}

function sf_format_object_details(&$data) {
  $message = "Object Details\n\r";
  $message .= "---------------------------------------------------------\n\r\n\r";

  foreach ($data as $field => $value) {
    $message .= $field . ": " . $value . "\n\r";
  }

  return $message;
}

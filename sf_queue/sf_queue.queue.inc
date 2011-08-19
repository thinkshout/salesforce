<?php
/**
 * @file
 *
 * Queue class definitions for the Salesforce Export Queue module.
 */

class SalesforceExportQueue extends SystemQueue {

  /**
   * Override SystemQueue's createItem() function to merge SF objects being
   * pushed with an existing record with that SFID already in the queue.
   */
  public function createItem($data) {
    $item = new stdClass();
    $item->item_id = $this->id_sequence++;
    $item->data = $data;
    $item->created = time();
    $item->expire = 0;
    $this->queue[$item->item_id] = $item;
  }
}
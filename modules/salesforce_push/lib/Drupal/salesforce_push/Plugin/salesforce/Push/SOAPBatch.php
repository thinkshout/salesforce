<?php

/**
 * @file
 * Contains \Drupal\salesforce_push\Plugin\salesforce\Push\SOAPBatch.
 */

namespace Drupal\salesforce_push\Plugin\salesforce\Push;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\salesforce_push\Plugin\PushPluginBase;

/**
 * @Plugin(
 *   id = "SOAPBatch",
 *   label = @Translation("SOAP Batch")
 * )
 */
class SOAPBatch extends PushPluginBase {

  protected function queue($op) {
    // Create queues by fieldmap name for increased efficiency of API ops. Queue
    // name can be up to 255 characters, so we have room for lots of data.
    // @see system.install
    // @todo determine optimal naming convention w.r.t API usage efficiency.
    // prefix + uuid + $op + delims = about 64 chars; plenty more room
    $queue = \Drupal::queue("salesforce_push:{$this->mapping->uuid()}:$op");
    $queue->createItem(array(
      'entity_type' => $this->entity->entityType(),
      'entity_id' => $this->entity->id(),
      'mapping_id' => $this->mapping->id(),
      'trigger' => $op,
    ));
  }

  public function push_create() {
    // Always use upsert because it is idempotent.
    $this->queue('upsert');
  }

  public function push_update() {
    // Always use upsert because it is idempotent.
    $this->queue('upsert');
  }

  public function push_delete() {
    if (empty($this->mapped_object->salesforce_id)) {
      // Can't delete a record without an id
      $this->mapped_object->delete();
    }
    else {
      $this->queue('delete');
    }
  }
}
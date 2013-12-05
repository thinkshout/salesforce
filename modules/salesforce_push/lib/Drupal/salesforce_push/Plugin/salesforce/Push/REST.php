<?php

/**
 * @file
 * Contains \Drupal\salesforce_push\Plugin\salesforce\Push\REST.
 */

namespace Drupal\salesforce_push\Plugin\salesforce\Push;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\salesforce_push\Plugin\PushPluginBase;

/**
 * @Plugin(
 *   id = "REST",
 *   label = @Translation("REST")
 * )
 */
class REST extends PushPluginBase {

  /**
   * @{inheritdoc}
   * @throws SalesforcException
   */
  public function push_create() {
    $this->push();
  }

  /**
   * @{inheritdoc}
   * @throws SalesforcException
   */
  public function push_update() {
    // No difference between drupal create and drupal update... yet
    $this->push();
  }

  /**
   * Wrapper for SalesforceClient->delete()
   * @throws SalesforceException
   */
  public function push_delete() {
    if (!$this->get_mapped_object()) {
      return FALSE;
    }
    // Delete API method returns nothing.
    // May throw an exception, which we allow to bubble up to the caller.
    $this->sf_client->objectDelete($this->mapping->salesforce_object_type, $this->mapped_object->sfid());
    salesforce_set_message(t('Salesforce object %sfid has been deleted.', array('%sfid' => $this->mapped_object->sfid())));
    $this->mapped_object->delete();
  }

  private function push() {
    if (empty($this->mapped_object)) {
      $this->push_unmapped();
    }
    else {
      $this->push_mapped();
    }

    // If we get this far, there was no error.
    salesforce_set_message(t('%name has been synchronized with Salesforce record %sfid.', array(
      '%name' => $this->entity->label(),
      '%sfid' => $this->mapped_object->sfid(),
    )));
    $this->mapped_object->entity_updated = $this->entity->get('changed');
    $this->mapped_object->save();
  }

  private function push_mapped() {
    // In case of mapped objects last sync is more recent than the entity's
    // timestamp, do not push an old change.
    if ($this->mapped_object->last_sync > $this->mapped_object->entity_updated) {
      return;
    }

    if ($this->mapping->hasKey()) {
      $this->_upsert();
    }
    else {
      $this->_update();
    }
    $this->mapped_object->last_sync = REQUEST_TIME;
  }

  private function push_unmapped() {
    // An external key has been specified, attempt an upsert().
    if ($this->mapping->hasKey()) {
      $sfid = $this->_upsert();
    }
    // No key or mapping, create a new object in Salesforce.
    else {
      $sfid = $this->_create();
    }

    // If we get this far, there was no error and sfid must have been assigned.
    // Create mapping object, saved in caller.
    // @todo use a mapping object factory
    $this->mapped_object = entity_create('salesforce_mapping_object', array(
      'entity_id' => $this->entity->id(),
      'entity_type' => $this->entity->entityType(),
      'salesforce_id' => $sfid,
    ));
  }

  /**
   * Underscore prefix so we don't conflict with any Drupal or Synfony voodoo.
   * Wrapper for SFAPI create()
   * @return salesforce id
   */
  private function _create() {
    $data = $this->sf_client->objectCreate($this->mapping->salesforce_object_type, $this->mapping->getPushParams($this->entity));
    return $data['id'];
  }

  /**
   * Underscore prefix so we don't conflict with any Drupal or Synfony voodoo.
   * Wrapper for SFAPI update() with some extra sauce.
   * @return null
   */
  private function _update() {
    try {
      $data = $this->sf_client->objectUpdate($this->mapping->salesforce_object_type, $this->mapped_object->sfid(), $this->mapping->getPushParams($this->entity));
    }
    catch(SalesforceException $e) {
      // @todo reconsider whether we really want to delete the mapping here.
      // e.g. Probably shouldn't delete a mapping for response code 500
      $this->mapped_object->delete();
      salesforce_set_message(t('Error message: "@msg".  The Salesforce link has been removed.', array('@msg' => $e->getMessage())), 'error');
      throw $e;
    }
  }

  /**
   * Underscore prefix so we don't conflict with any Drupal or Synfony voodoo.
   * Wrapper for SFAPI upsert() with some extra sauce.
   * @return salesforce id
   */
  private function _upsert() {
    // An upsert only returns an Id if the record was created. If no Id was
    // returned, go and fetch the Id. @see https://drupal.org/node/1992260
    $data = $this->sf_client->objectUpsert($this->mapping->salesforce_object_type, $this->mapping->getKeyField(), $this->mapping->getKeyValue($this->entity), $this->mapping->getPushParams($this->entity));
    if (empty($data['id'])) {
      $data = $this->sf_client->objectReadKey($this->mapping->salesforce_object_type, $this->mapping->getKeyField(), $this->mapping->getKeyValue($this->entity));
    }
    return $data['id'];
  }

}
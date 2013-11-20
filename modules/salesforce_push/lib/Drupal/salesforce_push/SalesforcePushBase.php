<?php

/**
 * @file
 * Contains \Drupal\salesforce_push\SalesforcePushBase.
 */

namespace Drupal\salesforce_push;

use Drupal\Core\Entity\EntityInterface;
use Drupal\salesforce_mapping\Entity\SalesforceMapping;
use Drupal\salesforce_mapping\Entity\SalesforceMappingObject;

abstract class SalesforcePushBase implements SalesforcePushInterface {

  protected $entity;
  protected $mapping;
  protected $mapped_object;

  public function __construct(EntityInterface $entity, SalesforceMapping $mapping) {
    $this->entity = $entity;
    $this->mapping = $mapping;
    $this->sf_client = \Drupal::service('salesforce.client');
    if (!$this->sf_client->isAuthorized()) {
      // Abort early if we can't do anything. Allows frees us from calling
      // isAuthorized() over and over.
      throw new SalesforceException('Salesforce needs to be authorized to connect to this website.');
    }
    $this->get_mapped_object();
  }

  // @todo this is ugly. If mapping objects were field-based, they'd already be attached to the entity.
  protected function get_mapped_object() {
    $this->mapped_object = salesforce_mapping_object_load_by_entity($this->entity);
    return !empty($this->mapped_object);
  }


}
<?php

/**
 * @file
 * Contains \Drupal\salesforce_push\SalesforcePushInterface.
 */

namespace Drupal\salesforce_push;

use Drupal\Core\Entity\EntityInterface;
use Drupal\salesforce_mapping\Entity\SalesforceMapping;

Interface SalesforcePushInterface {

  public function __construct(EntityInterface $entity, SalesforceMapping $mapping);

  public function push_create();

  public function push_update();

  public function push_delete();
  
}

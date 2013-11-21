<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\SalesforceMappingAccessController
 */

namespace Drupal\salesforce_mapping;

use Drupal\Core\Entity\EntityAccessController;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Session\AccountInterface;
use Drupal\salesforce\SalesforceClient;

/**
 * Access controller for the salesforce_mapping entity.
 *
 * @see \Drupal\salesforce_mapping\Entity\SalesforceMapping.
 */
class SalesforceMappingAccessController extends EntityAccessController {

  /**
   * {@inheritdoc}
   */
  protected function checkAccess(EntityInterface $entity, $operation, $langcode, AccountInterface $account) {
    switch ($operation) {
      case 'view':
        return $account->hasPermission('view salesforce mapping');
      default:
        // Apparently access controllers don't support dependency injection.
        if (\Drupal::service('salesforce.client')->isAuthorized()) {
          return $account->hasPermission('administer salesforce mapping');
        }
    }
  }
}

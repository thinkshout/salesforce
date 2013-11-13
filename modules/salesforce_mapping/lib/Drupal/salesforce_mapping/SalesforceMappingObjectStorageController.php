<?php

/**
 * @file
 * Definition of Drupal\salesforce_mapping\SalesforceMappingObjectStorageController.
 */

namespace Drupal\salesforce_mapping;

use Drupal\Core\Entity\FieldableDatabaseStorageController;
use Drupal\Core\Entity\EntityInterface;

/**
 * Controller class for Salesforce Mapping Objects.
 *
 * This extends the Drupal\Core\Entity\DatabaseStorageController class, adding
 * required special handling for salesforce mapping objects
 */
class SalesforceMappingObjectStorageController extends FieldableDatabaseStorageController {

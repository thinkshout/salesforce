<?php

/**
 * @file
 * Definition of \Drupal\salesforce\Entity\SObject.
 */

namespace Drupal\salesforce\Entity;

/**
 * An object representing a Salesforce Data Record
 * Adapted for Drupal from Salesforce PHP Toolkit
 * @see https://github.com/developerforce/Force.com-Toolkit-for-PHP
 * @todo figure out if this is the right approach
 * @todo implement a storage controller that makes sense
 * @EntityType(
 *   id = "sobject",
 *   label = @Translation("Salesforce Object"),
 *   controllers = {
 *     "storage" = "Drupal\file\NullStorage",
 *     "view_builder" = "Drupal\Core\Entity\EntityViewBuilder"
 *   },
 *   entity_keys = {
 *      "id" = "id",
 *      "bundle" = "type"
 *   }
 * )
 * @todo we'll probably need Dependency Injection of salesforce.client service
 */
class SObject extends Entity {
  /**
   * The Salesforce ID of this SObject
   */
  public $id;

  /**
   * The fields for this SObject
   */
  public $fields;

  /**
   * The Salesforce Object Type
   */
  public $type;

  /**
   * SFID is universally unique across all SF instances.
   */
  public function uuid() {
    return $this->id();
  }

  /**
   * The URI for SObjects is external
   */
  public function uri($rel = 'cannonical') {
    // @todo get the SF instance_url and build the correct URL here:
    // @todo respet the $rel param
    return 'http://www.salesforce.com/' . $this->id();
  }

  /**
   * {@inheritdoc}
   */
  protected function uriPlaceholderReplacements() {
    if (empty($this->uriPlaceholderReplacements)) {
      $this->uriPlaceholderReplacements = array(
        '{salesforce}' => $this->id(),
        '{salesforce_type}' => $this->bundle(),
      );
    }
    return $this->uriPlaceholderReplacements;
  }
}


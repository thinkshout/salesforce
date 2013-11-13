<?php

/**
 * @file
 * Contains \Drupal\salesforce\Plugin\DataType\SalesforceReference
 */

namespace Drupal\salesforce\Plugin\DataType\;

use Drupal\Core\TypedData\Annotation\DataType;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\TypedData\DataReferenceBase;

/**
 * Defines a 'salesforce_reference' data type.
 *
 * This serves as 'salesforce' property of salesforce reference field items.
 *
 * The plain value of this reference is a sobject record.
 * For setting the value, the salesforce object record or the salesforce ID may
 * be passed.
 *
 * Some supported constraints (below the definition's 'constraints' key) are:
 *  - Bundle: (optional) The bundle (salesforce object type) or an array of
 *              possible bundles.
 *
 * @DataType(
 *   id = "salesforce_reference",
 *   label = @Translation("Salesforce reference")
 * )
 */
class SalesforceReference extends DataReferenceBase {

  /**
   * The Salesforce ID
   *
   * @var string
   */
  protected $sfid;

  // @todo add useful things like object type and/or record type, and an actual reference to the data instead of just the ID
  
  /**
   * {@inheritdoc}
   */
  public function getTargetDefinition() {
    return array(
      'type' => 'sobject',
    );
  }

  /**
   * {@inheritdoc}
   * @todo retrieve the Salesforce Object record from cache instead of just the id.
   */
  public function getTarget() {
    return $this->sfid;
  }

  /**
   * {@inheritdoc}
   */
  public function getTargetIdentifier() {
    return $this->sfid;
  }

}
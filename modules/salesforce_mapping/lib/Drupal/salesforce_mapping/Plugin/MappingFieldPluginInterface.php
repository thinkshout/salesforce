<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\MappingFieldPluginInterface.
 */

namespace Drupal\salesforce_mapping\Plugin;
use Drupal\Core\Entity\EntityInterface;

/**
 * Defines an interface for salesforce mapping plugins.
 */
interface MappingFieldPluginInterface {

  /**
   * Returns label of the tip.
   *
   * @return string
   *   The label of the tip.
   */
  public function label();

  /**
   * Used for returning values by key.
   *
   * @var string
   *   Key of the value.
   *
   * @return string
   *   Value of the key.
   */
  public function get($key);

  /**
   * Used for returning values by key.
   *
   * @var string
   *   Key of the value.
   *
   * @var string
   *   Value of the key.
   */
  public function set($key, $value);

  /**
   * Given a Drupal entity, return the outbound value.
   */
  public function value(EntityInterface $entity);

}

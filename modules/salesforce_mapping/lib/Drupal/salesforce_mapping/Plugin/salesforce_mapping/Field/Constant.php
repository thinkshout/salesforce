<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field\Constant.
 */

namespace Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\salesforce_mapping\Plugin\FieldPluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
// use Drupal\Core\Utility\Constant;
use Symfony\Component\DependencyInjection\ContainerInterface;


/**
 * Adapter for entity Constant and fields.
 *
 * @Plugin(
 *   id = "Constant",
 *   label = @Translation("Constant")
 * )
 */
class Constant extends FieldPluginBase {
  
}
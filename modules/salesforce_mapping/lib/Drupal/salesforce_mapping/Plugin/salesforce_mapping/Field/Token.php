<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field\Token.
 */

namespace Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\salesforce_mapping\Plugin\FieldPluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
// use Drupal\Core\Utility\Token;
use Symfony\Component\DependencyInjection\ContainerInterface;


/**
 * Adapter for entity Token and fields.
 *
 * @Plugin(
 *   id = "Token",
 *   label = @Translation("Token")
 * )
 */
class Token extends FieldPluginBase {
  
}
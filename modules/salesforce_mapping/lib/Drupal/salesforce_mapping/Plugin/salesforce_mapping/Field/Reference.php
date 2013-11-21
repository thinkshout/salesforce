<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field\Reference.
 */

namespace Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\salesforce_mapping\Plugin\FieldPluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
// use Drupal\Core\Utility\Reference;
use Symfony\Component\DependencyInjection\ContainerInterface;


/**
 * Adapter for entity Reference and fields.
 *
 * @Plugin(
 *   id = "Reference",
 *   label = @Translation("Reference")
 * )
 */
class Reference extends FieldPluginBase {

  public $entityManager;
  
}
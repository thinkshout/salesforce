<?php

/**
 * @file
 * Contains \Drupal\salesforce_push\Plugin\PushPluginBase.
 */

namespace Drupal\salesforce_push\Plugin;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\Plugin\PluginBase;
use Drupal\salesforce\SalesforceClient;
use Drupal\salesforce_mapping\Entity\SalesforceMapping;
use Drupal\salesforce_mapping\Entity\SalesforceMappingObject;
use Drupal\salesforce_push\Plugin\PushPluginInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

abstract class PushPluginBase extends PluginBase implements PushPluginInterface, ContainerFactoryPluginInterface {

  protected $entity;
  protected $mapping;
  protected $sf_client;
  protected $mapped_object;

  protected $entity_manager;

  public function __construct(array $configuration, $plugin_id, array $plugin_definition, EntityManagerInterface $entity_manager, SalesforceClient $sf_client) {
    // dpm($configuration, $plugin_id, $plugin_definition);
    $this->sf_client = $sf_client;
    $this->entity_manager = $entity_manager;
    if (!$this->sf_client->isAuthorized()) {
      // Abort early if we can't do anything. Allows frees us from calling
      // isAuthorized() over and over.
      throw new SalesforceException('Salesforce needs to be authorized to connect to this website.');
    }
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, array $plugin_definition) {
    return new static($configuration, $plugin_id, $plugin_definition,
      $container->get('entity.manager'),
      $container->get('salesforce.client')
    );
  }

  public function init(EntityInterface $entity, SalesforceMapping $mapping) {
    $this->entity = $entity;
    $this->mapping = $mapping;
    $this->mapped_object = salesforce_mapping_object_load_by_entity($entity);
    return $this;
  }

}
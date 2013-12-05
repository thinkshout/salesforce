<?php

/**
 * @file
 * Contains \Drupal\salesforce_push\Plugin\PushPluginInterface.
 */

namespace Drupal\salesforce_push\Plugin;

use Drupal\Core\Entity\EntityInterface;
use Drupal\salesforce\SalesforceClient;
use Drupal\Core\Entity\EntityManagerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

interface PushPluginInterface {

  public function __construct(array $configuration, $plugin_id, array $plugin_definition, EntityManagerInterface $entity_manager, SalesforceClient $sf_client);

  public function push_create();

  public function push_update();

  public function push_delete();
  
}

<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\MappingFieldPluginBase.
 */

namespace Drupal\salesforce_mapping\Plugin;

use Drupal\Component\Plugin\ConfigurablePluginInterface;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Plugin\PluginFormInterface;
use Drupal\Core\Plugin\PluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\salesforce_mapping\Plugin\MappingFieldPluginInterface;
use Drupal\salesforce_mapping\Entity\SalesforceMapping;

// use Drupal\Core\Utility\Token;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Defines a base Salesforce Mapping Field Plugin implementation.
 * Extenders need to implement MappingFieldPluginInterface::value() and
 * PluginFormInterface::buildConfigurationForm().
 * @see Drupal\salesforce_mapping\Plugin\MappingFieldPluginInterface
 * @see Drupal\Core\Plugin\PluginFormInterface
 */
abstract class MappingFieldPluginBase extends PluginBase implements MappingFieldPluginInterface, PluginFormInterface, ConfigurablePluginInterface, ContainerFactoryPluginInterface {

  protected $label;
  protected $id;
  protected $mapping;
  protected $entityManager;

  // @see MappingFieldPluginInterface::value()
  // public function value();

  // @see PluginFormInterface::buildConfigurationForm().
  // public function buildConfigurationForm(array $form, array &$form_state);

  /**
   * Constructs a \Drupal\salesforce_mapping\Plugin\MappingFieldPluginBase object.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param array $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\Core\Entity\EntityManagerInterface $mapping
   *   The entity manager to get the SF listing, mapped entity, etc.
   */
  public function __construct(array $configuration, $plugin_id, array $plugin_definition, EntityManagerInterface $entity_manager) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->entityManager = $entity_manager;
    if (!empty($configuration['mapping_name'])) {
      $this->mapping = $this->entityManager
        ->getStorageController('salesforce_mapping')
        ->load($configuration['mapping_name']);
    }
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, array $plugin_definition) {
    return new static($configuration, $plugin_id, $plugin_definition, $container->get('entity.manager'));
  }

  /**
   * {@inheritdoc}
   */
  public function getConfiguration() {
    return $this->configuration;
  }

  /**
   * {@inheritdoc}
   */
  public function setConfiguration(array $configuration) {
    $this->configuration = $configuration;
  }

  /**
   * In order to set a config value to null, use setConfiguration()
   */
  public function config($key, $value = NULL) {
    if ($value !== NULL) {
      $this->configuration[$key] = $value;
    }
    if (array_key_exists($key, $this->configuration)) {
      return $this->configuration[$key];
    }
    return NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    return array(
      'key' => FALSE,
      'direction' => SALESFORCE_MAPPING_DIRECTION_SYNC,
      'salesforce_field' => array(),
      'drupal_field_type' => $this->id,
      'drupal_field_value' => '',
      'locked' => FALSE,
      'mapping_name' => '',
    );
  }

  /**
   * Implements PluginFormInterface::validateConfigurationForm().
   */
  public function validateConfigurationForm(array &$form, array &$form_state) {
    
  }

  /**
   * Implements PluginFormInterface::submitConfigurationForm().
   */
  public function submitConfigurationForm(array &$form, array &$form_state) {
    
  }

  /**
   * Implements MappingFieldPluginInterface::label().
   */
  public function label() {
    return $this->get('label');
  }

  /**
   * Implements MappingFieldPluginInterface::get().
   */
  public function get($key) {
    return property_exists($this, $key) ? $this->$key : NULL;
  }

  /**
   * Implements MappingFieldPluginInterface::get().
   */
  public function set($key, $value) {
    $this->$key = $value;
  }

  /**
   * @return bool
   *  Whether or not this field should be pushed to Salesforce.
   * @todo This needs a better name. Could be mistaken for a verb.
   */
  public function push() {
    return in_array($this->config('direction'), array(SALESFORCE_MAPPING_DIRECTION_DRUPAL_SF, SALESFORCE_MAPPING_DIRECTION_SYNC));
  }

  /**
   * @return bool
   *  Whether or not this field should be pulled from Salesforce to Drupal.
   * @todo This needs a better name. Could be mistaken for a verb.
   */
  public function pull() {
    return in_array($this->config('direction'), array(SALESFORCE_MAPPING_DIRECTION_SYNC, SALESFORCE_MAPPING_DIRECTION_SF_DRUPAL));
  }

}

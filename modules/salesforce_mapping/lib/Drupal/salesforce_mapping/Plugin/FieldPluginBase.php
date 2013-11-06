<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\FieldPluginBase.
 */

namespace Drupal\salesforce_mapping\Plugin;

use Drupal\Component\Plugin\ConfigurablePluginInterface;
use Drupal\Core\Plugin\PluginFormInterface;
use Drupal\Core\Plugin\PluginBase;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\salesforce_mapping\Plugin\FieldPluginInterface;
use Drupal\salesforce_mapping\Entity\SalesforceMapping;
// use Drupal\Core\Utility\Token;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Defines a base tour implementation.
 */
abstract class FieldPluginBase extends PluginBase implements FieldPluginInterface, PluginFormInterface, ConfigurablePluginInterface, ContainerFactoryPluginInterface {

  protected $label;
  protected $id;
  protected $mapping;

  // /**
  //  * Constructs a \Drupal\salesforce_mapping\Plugin\FieldPluginBase object.
  //  *
  //  * @param array $configuration
  //  *   A configuration array containing information about the plugin instance.
  //  * @param string $plugin_id
  //  *   The plugin_id for the plugin instance.
  //  * @param array $plugin_definition
  //  *   The plugin implementation definition.
  //  * @param \Drupal\salesforce_mapping\Entity\SalesforceMapping $mapping
  //  *   The token service.
  //  */
  // public function __construct(array $configuration, $plugin_id, array $plugin_definition, SalesforceMapping $mapping) {
  //   parent::__construct($configuration, $plugin_id, $plugin_definition);
  //   $this->mapping = $mapping;
  // }
  // 
  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, array $plugin_definition) {
    return new static($configuration, $plugin_id, $plugin_definition);
  }

  /**
   * {@inheritdoc}
   */
  public function getConfiguration() {
    
  }

  /**
   * {@inheritdoc}
   */
  public function setConfiguration(array $configuration) {
    
  }

  /**
   * {@inheritdoc}
   */
  public function defaultConfiguration() {
    
  }

  /**
   * {@inheritdoc}
   */
  public function validateConfigurationForm(array &$form, array &$form_state) {
  }

  public function submitConfigurationForm(array &$form, array &$form_state) {
    
  }

  /**
   * Implements FieldPluginInterface::label().
   */
  public function label() {
    return $this->get('label');
  }

  /**
   * Implements FieldPluginInterface::buildOptionsForm().
   */
  public function buildConfigurationForm(array $form, array &$form_state) {
    // Force extending classes to build their own options.
    return array();
  }

  /**
   * Implements FieldPluginInterface::get().
   */
  public function get($key) {
    return property_exists($this, $key) ? $this->$key : NULL;
  }

  /**
   * Implements FieldPluginInterface::get().
   */
  public function set($key, $value) {
    $this->$key = $value;
  }
  
}

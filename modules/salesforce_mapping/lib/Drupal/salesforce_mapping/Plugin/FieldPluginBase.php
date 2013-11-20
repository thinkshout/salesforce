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
    return $this->configuration;
  }

  /**
   * {@inheritdoc}
   */
  public function setConfiguration(array $configuration) {
    $this->configuration = $configuration;
  }

  public function config($key) {
    if (array_key_exists($key, $this->configuration)) {
      return $this->configuration[$key];
    }
    return FALSE;
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
    );

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

  /**
   * @return bool
   *  Whether or not this field should be pushed to Salesforce.
   * @todo Does this need a better name? Could be mistaken for a verb.
   */
  public function push() {
    return in_array($this->config('direction'), array(SALESFORCE_MAPPING_DIRECTION_DRUPAL_SF, SALESFORCE_MAPPING_DIRECTION_SYNC));
  }

  /**
   * @return bool
   *  Whether or not this field should be pulled from Salesforce to Drupal.
   * @todo Does this need a better name? Could be mistaken for a verb.
   */
  public function pull() {
    return in_array($this->config('direction'), array(SALESFORCE_MAPPING_DIRECTION_SYNC, SALESFORCE_MAPPING_DIRECTION_SF_DRUPAL));
  }

  /** 
   * We are purposefully not implementing value() to force extenders of this
   * class to do it themselves. 
   * public function value() { }
   */

}

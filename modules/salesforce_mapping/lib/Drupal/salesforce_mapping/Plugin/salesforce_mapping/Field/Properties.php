<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field\Properties.
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
 * Adapter for entity properties and fields.
 *
 * @Plugin(
 *   id = "properties",
 *   label = @Translation("Properties")
 * )
 */
class Properties extends FieldPluginBase {

  public $entityManager;
  
  /**
   * Constructs a \Drupal\salesforce_mapping\Plugin\FieldPluginBase object.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param array $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\salesforce_mapping\Entity\SalesforceMapping $mapping
   *   The token service.
   */
  public function __construct(array $configuration, $plugin_id, array $plugin_definition, EntityManagerInterface $entity_manager) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->entityManager = $entity_manager;
  }
  
  /**
   * {@inheritdoc}
   */
   public static function create(ContainerInterface $container, array $configuration, $plugin_id, array $plugin_definition) {
    return new static($configuration, $plugin_id, $plugin_definition, $container->get('entity.manager'));
  }

  public function buildConfigurationForm(array $form, array &$form_state) {
    parent::buildConfigurationForm($form, $form_state);

    // @todo inspecting the form and form_state seems wrong
    $entity = $form['#entity'];
    $entity_info = $this->entityManager->getFieldDefinitions($entity->get('drupal_entity_type'), $entity->get('drupal_bundle'));
    // Discard all but keys and labels:
    array_walk($entity_info, function(&$value, $key) {
      $value = $value['label'];
    });
    return array(
      '#type' => 'select',
      '#options' => $entity_info,
      '#empty_option' => $this->t('- Select -'),
      '#default_value' => $this->config('drupal_field_value'),
    );
  }

  public function value(EntityInterface $entity) {
    return 'Foo';
  }
}
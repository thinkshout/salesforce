<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce\MappingField\Token.
 */

namespace Drupal\salesforce_mapping\Plugin\salesforce\MappingField;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Utility\Token as TokenService;
use Drupal\salesforce_mapping\Plugin\MappingFieldPluginBase;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Adapter for entity Token and fields.
 *
 * @Plugin(
 *   id = "Token",
 *   label = @Translation("Token")
 * )
 */
class Token extends MappingFieldPluginBase {
 
  protected $token;

  /**
   * Constructs a \Drupal\salesforce_mapping\Plugin\MappingFieldPluginBase object.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param array $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\Core\Entity\EntityManagerInterface $entity_manager
   *   Entity manager, for loading mappings and mapped entities.
   * @param \Drupal\Core\Utility\Token (as TokenService) $token
   *   The token service.
   */
  public function __construct(array $configuration, $plugin_id, array $plugin_definition, EntityManagerInterface $entity_manager, TokenService $token) {
    parent::__construct($configuration, $plugin_id, $plugin_definition, $entity_manager);
    $this->token = $token;
  }

  public static function create(ContainerInterface $container, array $configuration, $plugin_id, array $plugin_definition) {
    return new static($configuration, $plugin_id, $plugin_definition, 
      $container->get('entity.manager'),
      $container->get('token'));
  }

  public function buildConfigurationForm(array $form, array &$form_state) {
    // @todo expose token options on mapping form: clear, callback, sanitize
    return array(
      '#type' => 'textfield',
      '#default_value' => $this->config('drupal_field_value'),
      '#description' => $this->t('Enter a token to map a Salesforce field..'),
    );
  }

  public function value(EntityInterface $entity) {
    // Even though everything is an entity, some token functions expect to
    // receive the entity keyed by entity type.
    $text = $this->config('drupal_field_value');
    $data = array('entity' => $entity, $entity->entityType() => $entity);
    return $this->token->replace($text, $data);
  }

}

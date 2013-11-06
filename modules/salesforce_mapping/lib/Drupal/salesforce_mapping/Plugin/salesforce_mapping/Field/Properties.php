<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field\Properties.
 */

namespace Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\Entity\EntityManagerInterface;
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

  var $entityManager;

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
dpm($this->getConfiguration());
    // @todo inspecting the form and form_state seems wrong
    $entity = $form['#entity'];
    $entity_type = $entity->get('drupal_entity_type');
    $entity_info = $this->entityManager->getDefinition($entity_type);
    // @todo find a suitable replacement for entity_get_all_property_info()
    dpm($entity);
    dpm(func_get_args());
    return array(
      // '#title' => t('Drupal Field'),
      '#type' => 'select',
      '#options' => $entity_info['entity_keys'],
      '#empty_option' => $this->t('- Select -'),
    );
    
    // $entity_type = $this->mapping->get('drupal_entity_type');
    // 
    // $form['fieldmap_value'] = array(
    //   '#type' => 'select',
    //   '#title' => 'Select Property',
    //   '#description' => $this->t('Select a Drupal field or property to map to a Salesforce field. Related are left out and should be handled using another fieldmap type like tokens.'),
    // );
    // $options = array('' => $this->t('-- Select --'));
    // 
    // $entity_info = \Drupal::entityManager()->getDefinition($entity_type);
    // 
    // // @todo: figure out how to identify relevant properties
    // // @todo: add fields
    // // for now, just expose entity keys
    // foreach ($entity_info['entity_keys'] as $key => $property) {
    //   $options[$key] = $property;
    //   // $type = isset($property['type']) ? entity_property_extract_innermost_type($property['type']) : 'text';
    //   // $is_entity = ($type == 'entity') || (bool) entity_get_info($type);
    //   // // Leave entities out of this.
    //   // if (($is_entity && $include_entities) || (!$is_entity && !$include_entities)) {
    //   //   if (isset($property['field']) && $property['field'] && !empty($property['property info'])) {
    //   //     foreach ($property['property info'] as $sub_key => $sub_prop) {
    //   //       $options[$property['label']][$key . ':' . $sub_key] = $sub_prop['label'];
    //   //     }
    //   //   }
    //   //   else {
    //   //     $options[$key] = $property['label'];
    //   //   }
    //   // }
    // }
    // 
    // return $form;
  }
}
<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\SalesforceMappingListController.
 */

namespace Drupal\salesforce_mapping;

use Drupal\Component\Utility\String;
use Drupal\Core\Config\ConfigFactory;
use Drupal\Core\Config\Entity\DraggableListController;
use Drupal\Core\Entity\EntityControllerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityStorageControllerInterface;
use Drupal\Core\Extension\ModuleHandlerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Defines the filter format list controller.
 */
class SalesforceMappingListController extends DraggableListController implements EntityControllerInterface {

  // /**
  //  * {@inheritdoc}
  //  */
  // protected $entitiesKey = 'mappings';
  // 
  // /**
  //  * The config factory service.
  //  *
  //  * @var \Drupal\Core\Config\ConfigFactory
  //  */
  // protected $configFactory;
  // 
  // /**
  //  * Constructs a new SalesforceMappingListController.
  //  *
  //  * @param string $entity_type
  //  *   The type of entity to be listed.
  //  * @param array $entity_info
  //  *   An array of entity info for the entity type.
  //  * @param \Drupal\Core\Entity\EntityStorageControllerInterface $storage
  //  *   The entity storage controller class.
  //  * @param \Drupal\Core\Extension\ModuleHandlerInterface $module_handler
  //  *   The module handler to invoke hooks on.
  //  * @param \Drupal\Core\Config\ConfigFactory $config_factory
  //  *   The config factory.
  //  */
  // public function __construct($entity_type, array $entity_info, EntityStorageControllerInterface $storage, ModuleHandlerInterface $module_handler, ConfigFactory $config_factory) {
  //   parent::__construct($entity_type, $entity_info, $storage, $module_handler);
  // 
  //   $this->configFactory = $config_factory;
  // }

  /**
   * {@inheritdoc}
   */
  public static function createInstance(ContainerInterface $container, $entity_type, array $entity_info) {
    return new static(
      $entity_type,
      $entity_info,
      $container->get('entity.manager')->getStorageController($entity_type),
      $container->get('module_handler'),
      $container->get('config.factory')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'salesforce_mappings_list';
  }

  /**
   * {@inheritdoc}
   */
  // public function load() {
  //   // Only list enabled filters.
  //   return array_filter(parent::load(), function ($entity) {
  //     return $entity->status();
  //   });
  // }

  /**
   * {@inheritdoc}
   */
  public function buildHeader() {
    $header['label'] = t('Name');
    // $header['roles'] = t('Roles');
    return $header + parent::buildHeader();
  }
  // 
  // /**
  //  * {@inheritdoc}
  //  */
  public function buildRow(EntityInterface $entity) {
    $row = array();
    $row['label'] = $entity->label();
    return $row + parent::buildRow($entity);
  }
  // 
  // /**
  //  * {@inheritdoc}
  //  */
  // public function getOperations(EntityInterface $entity) {
  //   $operations = parent::getOperations($entity);
  // 
  //   if (isset($operations['edit'])) {
  //     $operations['edit']['title'] = t('Configure');
  //   }
  // 
  //   // The fallback format may not be disabled.
  //   if ($entity->isFallbackFormat()) {
  //     unset($operations['disable']);
  //   }
  // 
  //   return $operations;
  // }
  
  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $form = parent::buildForm($form, $form_state);
    $form['actions']['submit']['#value'] = t('Save changes');
    return $form;
  }
  // /**
  //  * {@inheritdoc}
  //  */
  // public function submitForm(array &$form, array &$form_state) {
  //   parent::submitForm($form, $form_state);
  // 
  //   filter_formats_reset();
  //   drupal_set_message(t('The text format ordering has been saved.'));
  // }

}

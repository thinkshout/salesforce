<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\SalesforceMappingAddFormController.
 */

namespace Drupal\salesforce_mapping\Form;

use Drupal\Core\Entity\EntityFormController;
use Drupal\Core\Entity\EntityStorageControllerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Salesforce Mapping UI controller.
 */
class SalesforceMappingAddFormController extends EntityFormController {

  /**
   * The storage controller.
   *
   * @var \Drupal\Core\Entity\EntityStorageControllerInterface
   */
  protected $storageController;

  /**
   * Constructs a new FilterFormatFormControllerBase.
   *
   * @param \Drupal\Core\Config\ConfigFactory $config_factory
   *   The config factory.
   * @param \Drupal\Core\Entity\Query\QueryFactory $query_factory
   *   The entity query factory.
   */
  public function __construct(EntityStorageControllerInterface $storageController) {
    $this->storageController = $storageController;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('entity.manager')->getStorageController('salesforce_mapping')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function form(array $form, array &$form_state) {
    // watchdog('foo', ddebug_backtrace(TRUE));
    $mapping = $this->entity;
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label'),
      '#default_value' => $mapping->label(),
      '#required' => TRUE,
      '#weight' => -30,
    );
    $form['id'] = array(
      '#type' => 'machine_name',
      '#required' => TRUE,
      '#default_value' => $mapping->id(),
      '#maxlength' => 255,
      '#machine_name' => array(
        'exists' => array($this->storageController, 'load'),
        'source' => array('label'),
      ),
      '#disabled' => !$mapping->isNew(),
      '#weight' => -20,
    );

    return parent::form($form, $form_state);
  }

 // /**
 //   * {@inheritdoc}
 //   */
 //  public function validate(array $form, array &$form_state) {
 //    // parent::validate($form, $form_state);
 //    // 
 //    // if ($this->plugin instanceof PluginFormInterface) {
 //    //   $this->plugin->validateConfigurationForm($form, $form_state);
 //    // }
 //  }
 // 
 //  /**
 //   * {@inheritdoc}
 //   */
 //  public function submit(array $form, array &$form_state) {
 //    // parent::submit($form, $form_state);
 //    // 
 //    // if ($this->plugin instanceof PluginFormInterface) {
 //    //   $this->plugin->submitConfigurationForm($form, $form_state);
 //    // }
 //    return $this->entity;
 //  }
 // 
 //  /**
 //   * {@inheritdoc}
 //   */
  public function save(array $form, array &$form_state) {
dpm(func_get_args());
dpm($this);
    dpm($this->entity);
    $this->entity->save();
    drupal_set_message($this->t('The action has been successfully saved.'));
 
    $form_state['redirect_route'] = array(
      'route_name' => 'salesforce_mapping.list',
    );
  }
}

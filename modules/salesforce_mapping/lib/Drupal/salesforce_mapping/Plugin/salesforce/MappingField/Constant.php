<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce\MappingField\Constant.
 */

namespace Drupal\salesforce_mapping\Plugin\salesforce\MappingField;

use Drupal\Component\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
// use Drupal\Core\Utility\Constant;
use Drupal\Core\Utility\Token;
use Drupal\salesforce_mapping\Plugin\MappingFieldPluginBase;
use Symfony\Component\DependencyInjection\ContainerInterface;


/**
 * Adapter for entity Constant and fields.
 *
 * @Plugin(
 *   id = "Constant",
 *   label = @Translation("Constant")
 * )
 */
class Constant extends MappingFieldPluginBase {

  public function buildConfigurationForm(array $form, array &$form_state) {
    return array(
      '#type' => 'textfield',
      '#default_value' => $this->config('drupal_field_value'),
      '#description' => $this->t('Enter a constant value to map to a Salesforce field.'),
    );
  }

  public function value(EntityInterface $entity) {
    // @todo token replace goes here:
    return $this->config('drupal_field_value');
  }

  // @todo add validation handler. Prevent user from submitting anything that
  // isn't a token.

}
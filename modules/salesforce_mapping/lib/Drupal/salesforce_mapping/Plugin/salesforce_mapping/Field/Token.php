<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Plugin\salesforce_mapping\Field\Token.
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
 * Adapter for entity Token and fields.
 *
 * @Plugin(
 *   id = "Token",
 *   label = @Translation("Token")
 * )
 */
class Token extends FieldPluginBase {
 
  public function buildConfigurationForm(array $form, array &$form_state) {
    return array(
      '#type' => 'textfield',
      '#default_value' => $this->config('drupal_field_value'),
      '#description' => $this->t('Enter a token to map a Salesforce field..'),
    );
  }

  public function value(EntityInterface $entity) {
    return $this->config('drupal_field_value');
  }
 
}
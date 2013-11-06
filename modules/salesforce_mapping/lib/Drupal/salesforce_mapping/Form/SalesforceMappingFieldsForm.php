<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\SalesforceMappingFieldsForm.
 */

namespace Drupal\salesforce_mapping\Form;

use Symfony\Component\Debug\Debug;

use Drupal\Component\Utility\NestedArray;
use Drupal\Core\Ajax\CommandInterface;
use Drupal\Core\Ajax\AjaxResponse;
use Drupal\Core\Ajax\ReplaceCommand;
use Drupal\Core\Ajax\InsertCommand;
use Drupal\Core\Config\ConfigFactory;
use Drupal\Core\Config\Context\ContextInterface;
use Drupal\salesforce_mapping\Plugin\FieldPluginInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Salesforce Mapping Fields Form .
 */
class SalesforceMappingFieldsForm extends SalesforceMappingFormBase {

  /**
   * Stores the configuration factory.
   *
   * @var \Drupal\Core\Config\ConfigFactory
   */
  protected $configFactory;

  /**
   * Salesforce Mapping Plugin Field manager
   *
   * @var \Drupal\salesforce_mapping\Plugin\FieldPluginInterface
   */
  protected $fieldManager;

  /**
   * Constructs a \Drupal\system\ConfigFormBase object.
   *
   * @param \Drupal\Core\Config\ConfigFactory $config_factory
   *   The factory for configuration objects.
   * @param \Drupal\Core\Config\Context\ContextInterface $context
   *   The configuration context to use.
   */
  public function __construct(ConfigFactory $config_factory, ContextInterface $context, FieldPluginInterface $field_manager) {
    $this->configFactory = $config_factory;
    $this->configFactory->enterContext($context);
    $this->fieldManager = $field_manager;

    Debug::enable(E_ALL);
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('config.factory'),
      $container->get('config.context.free'),
      $container->get('plugin.manager.salesforce_mapping.field')
    );
  }


  /**
   * Previously "Field Mapping" table on the map edit form.
   * {@inheritdoc}
   * @todo add a header with Fieldmap Property information
   */
  public function form(array $form, array &$form_state) {
    $form['#entity'] = $this->entity;
    // For each field on the map, add a row to our table.
    $form['overview'] = array('#markup' => 'Field mapping overview goes here.');
    $form['salesforce_field_mappings_wrapper'] = array(
      '#title' => t('Field map'),
      '#type' => 'fieldset',
      '#id' => 'edit-salesforce-field-mappings-wrapper',
      '#description' => '* Key refers to an property mapped to a Salesforce external ID. if specified an UPSERT will be used to avoid duplicate data when possible.',
    );

    $field_mappings_wrapper = &$form['salesforce_field_mappings_wrapper'];
    // Check to see if we have enough information to allow mapping fields.  If
    // not, tell the user what is needed in order to have the field map show up.

    $field_mappings_wrapper['salesforce_field_mappings'] = array(
      '#tree' => TRUE,
      '#type' => 'table',
      '#header' => array(
        'field_type' => '',
        'drupal_field' => t('Drupal field'),
        'salesforce_field' => t('Salesforce field'),
        'key' => t('Key') . '*',
        'direction' => t('Direction'),
      ),
      '#attributes' => array(
        'id' => array('edit-salesforce-field-mappings'),
      ),
    );
    $rows = &$field_mappings_wrapper['salesforce_field_mappings'];

    $form['salesforce_field_mappings_wrapper']['ajax_warning'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => array('edit-ajax-warning'),
      ),
    );

    // @todo figure out how D8 does tokens
    // $form['salesforce_field_mappings_wrapper']['token_tree'] = array(
    //   '#type' => 'container',
    //   '#attributes' => array(
    //     'id' => array('edit-token-tree'),
    //   ),
    // );
    // $form['salesforce_field_mappings_wrapper']['token_tree']['tree'] = array(
    //   '#theme' => 'token_tree',
    //   '#token_types' => array($drupal_entity_type),
    //   '#global_types' => TRUE,
    // );
    $add_field_text = !empty($field_mappings) ? t('Add another field mapping') : t('Add a field mapping to get started');

    
    $form['buttons'] = array('#type' => 'container');
    $form['buttons']['field_type'] = array(
      '#title' => t('Field Type'),
      '#type' => 'select',
      '#options' => $this->get_drupal_type_options(),
      '#attributes' => array('id' => array('edit-salesforce-mapping-add-field-type'))
    );
    $form['buttons']['add'] = array(
      '#value' => $add_field_text,
      '#type' => 'submit',
      '#executes_submit_callback' => FALSE,
      '#limit_validation_errors' => array(),
      '#ajax' => array(
        'callback' => array($this, 'field_add_callback'),
        'wrapper' => 'edit-salesforce-field-mappings-wrapper',
      ),
      // @todo add validation to field_add_callback()
      '#states' => array(
        'disabled' => array(
          ':input#edit-salesforce-mapping-add-field-type' => array('value' => ''),
        ),
      ),
    );

    // Field mapping form.
    $field_mappings = array_filter($this->entity->get('field_mappings'));
    $has_token_type = FALSE;

    $delta = 0;
    // Add a row for each saved mapping
    foreach ($field_mappings as $delta => $value) {
      $value['delta'] = $delta;
      $rows[$delta] = $this->get_row($value, $form, $form_state);
    }

    // Apply any changes from form_state to existing fields.
    $input = array();
    if (!empty($form_state['input']['salesforce_field_mappings'])) {
      $input = &$form_state['input']['salesforce_field_mappings'];
    }
    while (isset($input[++$delta])) {
      $rows[$delta] = $this->get_row($input[$delta], $form, $form_state);
    }
    // @todo input does not contain the clicked button, have to go to values for
    // that. This may change?

    // Add button was clicked. See if we have a field_type value -- it's
    // required. If not, take no action. #states is already used to prevent
    // users from adding without selecting field_type. If they've worked
    // around that, they're going to have problems.
// error_log(print_r($form_state['values'], 1));
// error_log(print_r($form_state['input'], 1));
    if (!empty($form_state['values'])
    && $form_state['values']['add'] == $form_state['values']['op']
    && !empty($form_state['input']['field_type'])) {
      $rows[$delta] = $this->get_row(array('drupal_field' => array('fieldmap_type' => $form_state['input']['field_type'])), $form, $form_state);
      $form['buttons']['field_type']['#default_value'] = '';
    }

// ob_start();
// var_dump($form);
// error_log(ob_get_clean());
    // Add any fields which are in form_state but not yet saved.

    return parent::form($form, $form_state);
  }

  /**
   * Helper function to return an empty row for the field mapping form.
   *
   * @author Aaron Bauman
   */
  private function get_row($field = array(), $form, $form_state) {
    // @todo this is already defined in schema.yml: can we use that or a
    // settings.yml file instead of rewriting?
    $defaults = array(
      'delta' => 'new',
      'key' => FALSE,
      'direction' => SALESFORCE_MAPPING_DIRECTION_SYNC,
      'salesforce_field' => array(),
      'drupal_field' => array(
        'fieldmap_type' => '', 
        'fieldmap_value' => ''
      ),
    );
    $field = NestedArray::mergeDeepArray(array($defaults, $field));

    $field_plugin_definition = $this->get_field_plugin($field['drupal_field']['fieldmap_type']);
    $row['field_type'] = array(
        '#title' => 'Field Type',
        '#type' => 'item',
        // @todo replace with label from plugin:
        '#markup' => $field_plugin_definition['label'],
    );
    $field_plugin = $this->fieldManager->createInstance($field_plugin_definition['id']);
    dpm($field_plugin);
    // This should be the field-type specific values.
    $row['drupal_field'] = array(
      'fieldmap_type' => array(
        '#type' => 'hidden',
        '#value' => $field['drupal_field']['fieldmap_type'],
      ),
      // Display the plugin config form here:
      'fieldmap_value' => $field_plugin->buildConfigurationForm($form, $form_state),
    );

    $row['salesforce_field'] = array(
      '#title' => t('Salesforce Field'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce field to map.'),
      '#multiple' => (isset($fieldmap_type['salesforce_multiple_fields']) && $fieldmap_type['salesforce_multiple_fields']) ? TRUE : FALSE,
      '#options' => $this->get_salesforce_field_options(),
      '#default_value' => $field['salesforce_field'],
    );

    $row['key'] = array(
      '#name' => 'key',
      '#title' => t('Key'),
      '#type' => 'radio',
      '#return_value' => $field['delta'],
      '#default_value' => $field['key'],
    );

    $row['direction'] = array(
      '#title' => t('Direction'),
      '#type' => 'radios',
      '#options' => array(
        SALESFORCE_MAPPING_DIRECTION_DRUPAL_SF => t('Drupal to SF'),
        SALESFORCE_MAPPING_DIRECTION_SF_DRUPAL => t('SF to Drupal'),
        SALESFORCE_MAPPING_DIRECTION_SYNC => t('Sync'),
      ),
      '#required' => TRUE,
      '#default_value' => $field['direction'],
    );
    return $row;
  }


 /**
   * {@inheritdoc}
   */
  public function validate(array $form, array &$form_state) {
    parent::validate($form, $form_state);
    // What do we need to do here?
  }
 
  /**
   * {@inheritdoc}
   */
  public function submit(array $form, array &$form_state) {
    parent::submit($form, $form_state);
    // What do we need to do here?
    return $this->entity;
  }

  public function field_add_callback($form, &$form_state) {
    $response = new AjaxResponse();
    // Requires updating itself and the field map.
    $response->addCommand(new ReplaceCommand('#edit-salesforce-field-mappings-wrapper', render($form['salesforce_field_mappings_wrapper'])));
    return $response;
  }

  protected function get_drupal_type_options($include_select = TRUE) {
    $field_plugins = $this->fieldManager->getDefinitions();
    dpm($field_plugins);
    $field_type_options = array();
    if ($include_select) {
      $field_type_options[''] = t('- Select -');
    }
    foreach ($field_plugins as $field_plugin) {
      $field_type_options[$field_plugin['id']] = $field_plugin['label'];
    }
    return $field_type_options;
  }

  protected function get_field_plugin($field_type) {
    // @todo not sure if it's best practice to static cache definitions, or just
    // get them from fieldManager each time.
    $field_plugins = $this->fieldManager->getDefinitions();
    return $field_plugins[$field_type];
  }

  /**
   * Helper to retreive a list of fields for a given object type.
   *
   * @param string $salesforce_object_type
   *   The object type of whose fields you want to retreive.
   *
   * @return array
   *   An array of values keyed by machine name of the field with the label as
   *   the value, formatted to be appropriate as a value for #options.
   */
  protected function get_salesforce_field_options($salesforce_object_type = '') {
    if (empty($salesforce_object_type)) {
      $salesforce_object_type = $this->entity->get('salesforce_object_type');
    }
    $sfobject = $this->get_salesforce_object($salesforce_object_type);
    $sf_fields = array('' => $this->t('- ' . t('Select') . ' -'));
    if (isset($sfobject['fields'])) {
      foreach ($sfobject['fields'] as $sf_field) {
        $sf_fields[$sf_field['name']] = $sf_field['label'];
      }
    }
    return $sf_fields;
  }

}

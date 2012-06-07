<?php

/**
 * @file
 * Commerce discount editing UI.
 */

/**
 * UI controller.
 */
class CommerceDiscountUIController extends EntityDefaultUIController {

  /**
   * Overridden to customize the field location.
   */
  public function entityFormSubmitBuildEntity($form, &$form_state) {
    // We cannot use entity_form_submit_build_entity() any more.
    $entity = $form_state['commerce_discount'];

    // Extract form values.
    form_state_values_clean($form_state);
    foreach ($form_state['values'] as $key => $value) {
      if ($key != 'commerce_discount_fields') {
        $entity->$key = $value;
      }
    }
    // Invoke all specified builders for copying form values to entity
    // properties.
    // @see entity_form_submit_build_entity()
    if (isset($form['#entity_builders'])) {
      foreach ($form['#entity_builders'] as $function) {
        $function('commerce_discount', $entity, $form, $form_state);
      }
    }
    field_attach_submit('commerce_discount', $entity, $form['commerce_discount_fields'], $form_state);
    return $entity;
  }

  /**
   * Overriden EntityDefaultUIController::hook_menu().
   *
   * Call our own page callback to show the "commerce_discount_overview"
   * view, as this view has exposed filters which will not work inside
   * Entity API's default form.
   */
  public function hook_menu() {
    $plural_label = isset($this->entityInfo['plural label']) ? $this->entityInfo['plural label'] : $this->entityInfo['label'] . 's';

    $items = parent::hook_menu();
    $items[$this->path] = array(
      'title' => $plural_label,
      'page callback' => 'commerce_discount_overview_form',
      'description' => 'Manage ' . $plural_label . '.',
      'access callback' => 'entity_access',
      'access arguments' => array('view', $this->entityType),
      'file' => 'includes/commerce_discount.admin.inc',
      'file path' => drupal_get_path('module', 'commerce_discount'),
    );
    return $items;
  }

}

/**
 * Builds the entity overview form.
 */
function commerce_discount_overview_form() {
  $view = views_get_view('commerce_discount_overview');
  $view->override_url = $_GET['q'];
  $form['view'] = array('#markup' => $view->preview());
  $form['#attached']['css'][] = drupal_get_path('module', 'commerce_discount') . '/css/commerce_discount.css';
  return $form;
}

/**
 * Form callback: create or edit a discount.
 *
 * @param $discount
 *   The discount type array to edit or for a create form an empty discount
 *   type array with properties instantiated but not populated.
 */
function commerce_discount_form($form, &$form_state, CommerceDiscount $commerce_discount, $op = 'edit') {
  // We might have gotten the commerce discount type via ajax, so set it
  // in the commerce discount entity.
  if (!empty($form_state['values']['commerce_discount_type'])) {
    $commerce_discount->type = $form_state['values']['commerce_discount_type'];
  }

  $options = array();
  foreach (commerce_discount_types() as $type => $info) {
    $options[$type] = $info['label'];
  }

  // Show a list of commerce discounts.
  $form['commerce_discount_type'] = array(
    '#title' => t('Choose promotion type'),
    '#type' => 'radios',
    '#options' => $options,
    '#required' => TRUE,
    '#default_value' => $commerce_discount->type,
    '#ajax' => array(
       'callback' => 'commerce_discount_fields_ajax_callback',
       'wrapper' => 'commerce-discount-fields-wrapper',
    ),
  );

  $form['label'] = array(
    '#title' => t('Admin title'),
    '#type' => 'textfield',
    '#default_value' => $commerce_discount->label,
    '#description' => t('Shown only on managment screens, not shown to customers.'),
    '#required' => TRUE,
  );

  // Machine-readable type name.
  $form['name'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($commerce_discount->name) ? $commerce_discount->name : '',
    '#disabled' => $commerce_discount->hasStatus(ENTITY_IN_CODE),
    '#machine_name' => array(
      'exists' => 'commerce_discount_load',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this message type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['commerce_discount_fields'] = array(
    '#prefix' => '<div id="commerce-discount-fields-wrapper">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#parents' => array('commerce_discount_fields'),
  );
  field_attach_form('commerce_discount', $commerce_discount, $form['commerce_discount_fields'], $form_state);

  $form['enabled'] = array(
    '#title' => t('Promotion status'),
    '#type' => 'radios',
    '#options' => array(
      TRUE => t('Enabled'),
      FALSE => t('Disabled'),
    ),
    '#required' => TRUE,
    '#default_value' => $commerce_discount->enabled,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save commerce discount'),
    '#weight' => 40,
  );

  if (!$commerce_discount->hasStatus(ENTITY_IN_CODE) && $op != 'add') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete commerce discount'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('commerce_discount_form_submit_delete')
    );
  }

  // Add assets.
  $form['#attached']['js'][] = drupal_get_path('module', 'commerce_discount') . '/js/commerce_discount.js';
  $form['#attached']['css'][] = drupal_get_path('module', 'commerce_discount') . '/css/commerce_discount.css';

  return $form;
}

/**
 * AJAX callback to attach the commerce discount fields to the form.
 *
 * Since the controlling logic for populating the form is in the form builder
 * function, all we do here is select the element and return it to be updated.
 */
function commerce_discount_fields_ajax_callback($form, &$form_state) {
  return $form['commerce_discount_fields'];
}

/**
 * Form API validation callback for the type form.
 */
function commerce_discount_form_validate($form, $form_state) {
  field_attach_form_validate('commerce_discount', $form_state['commerce_discount'], $form['commerce_discount_fields'], $form_state);
}

/**
 * Form API submit callback for the type form.
 */
function commerce_discount_form_submit(&$form, &$form_state) {
  $commerce_discount = entity_ui_form_submit_build_entity($form, $form_state);
  $commerce_discount->save();
  $form_state['redirect'] = 'admin/commerce/store/commerce-discounts';
}

/**
 * Form API submit callback for the delete button.
 */
function commerce_discount_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/commerce/store/commerce-discounts/manage/' . $form_state['commerce_discount']->name . '/delete';
}

<?php

/**
 * @file
 *   commerce_discount.admin.inc
 *
 * Commerce discount editing UI.
 */


/**
 * Form API submit callback for the type form.
 */
function commerce_discount_form_submit(&$form, &$form_state) {
  $commerce_discount = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  $commerce_discount->save();
  $form_state['redirect'] = 'admin/structure/commerce-discount';
}

/**
 * Form API submit callback for the delete button.
 */
function commerce_discount_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/commerce-discount/manage/' . $form_state['commerce_discount']->name . '/delete';
}

/**
 * Menu callback: display a list of discount types that the user can create.
 */
function commerce_discount_add_page() {
  $item = menu_get_item();
  $content = system_admin_menu_block($item);

  return theme('discount_add_list', array('content' => $content));
}

/**
 * Displays the list of available discount types for discount creation.
 *
 * @ingroup themeable
 */
function theme_discount_add_list($variables) {
  $content = $variables['content'];
  $output = '';

  if ($content) {
    $output = '<dl class="commerce-discount-type-list">';
    foreach ($content as $item) {
      $output .= '<dt>' . l($item['title'], $item['href'], $item['localized_options']) . '</dt>';
      $output .= '<dd>' . filter_xss_admin($item['description']) . '</dd>';
    }
    $output .= '</dl>';
  }
  else {
    if (user_access('administer discount types')) {
      $output = '<p>' . t('You have not created any discount types yet. Go to the <a href="@create-discount-type">discount type creation page</a> to add a new discount type.', array('@create-discount-type' => url('admin/structure/discounts/types/add'))) . '</p>';
    }
    else {
      $output = '<p>' . t('No discount types have been created yet for you to use.') . '</p>';
    }
  }

  return $output;
}

/**
 * Menu callback: display an overview of available types.
 */
function commerce_discount_types_overview() {
  $header = array(
    t('Name'),
    t('Operations'),
  );

  $rows = array();

  // Loop through all defined discount types.
  foreach (commerce_discount_types() as $type => $discount_type) {
    // Build the operation links for the current discount type.
    $links = menu_contextual_links('commerce-discount-type', 'admin/structure/commerce-discounts/types', array(strtr($type, array('_' => '-'))));

    // Add the discount type's row to the table's rows array.
    $rows[] = array(
      theme('discount_type_admin_overview', array('discount_type' => $discount_type)),
      theme('links', array('links' => $links, 'attributes' => array('class' => 'links inline operations'))),
    );
  }

  // If no discount types are defined...
  if (empty($rows)) {
    // Add a standard empty row with a link to add a new discount type.
    $rows[] = array(
      array(
        'data' => t('There are no discount types yet. <a href="@link">Add discount type</a>.', array('@link' => url('admin/structure/commerce-discounts/types/add'))),
        'colspan' => 2,
      )
    );
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Builds an overview of a discount type for display to an administrator.
 *
 * @param $variables
 *   An array of variables used to generate the display; by default includes the
 *     type key with a value of the discount type array.
 *
 * @ingroup themeable
 */
function theme_discount_type_admin_overview($variables) {
  $discount_type = $variables['discount_type'];

  $output = check_plain($discount_type['name']);
  $output .= ' <small>' . t('(Machine name: @type)', array('@type' => $discount_type['type'])) . '</small>';

  return $output;
}

/**
 * Form callback: create or edit a discount type.
 *
 * @param $discount_type
 *   The discount type array to edit or for a create form an empty discount type
 *     array with properties instantiated but not populated.
 */
function commerce_discount_discount_type_form($form, &$form_state, $discount_type) {
  // Ensure this include file is loaded when the form is rebuilt from the cache.
  $form_state['build_info']['files']['form'] = drupal_get_path('module', 'commerce_discount') . '/includes/commerce_discount.forms.inc';

  // Store the initial discount type in the form state.
  $form_state['discount_type'] = $discount_type;

  $form['discount_type'] = array(
    '#tree' => TRUE,
  );

  $form['discount_type']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $discount_type['name'],
    '#description' => t('The human-readable name of this discount type. It is recommended that this name begin with a capital letter and contain only letters, numbers, and spaces. This name must be unique.'),
    '#required' => TRUE,
    '#size' => 32,
  );

  if (empty($discount_type['type'])) {
    $form['discount_type']['type'] = array(
      '#type' => 'machine_name',
      '#title' => t('Machine name'),
      '#default_value' => $discount_type['type'],
      '#maxlength' => 32,
      '#required' => TRUE,
      '#machine_name' => array(
        'exists' => 'commerce_discount_type_load',
        'source' => array('discount_type', 'name'),
      ),
      '#description' => t('The machine-readable name of this discount type. This name must contain only lowercase letters, numbers, and underscores, it must be unique.'),
    );
  }

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 40,
  );

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = array();

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save discount type'),
    '#submit' => array_merge($submit, array('commerce_discount_discount_type_form_submit')),
  );

  if (!empty($form_state['discount_type']['type'])) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete discount type'),
      '#suffix' => l(t('Cancel'), 'admin/structure/commerce-discounts/types'),
      '#submit' => array_merge($submit, array('commerce_discount_discount_type_form_delete_submit')),
      '#weight' => 45,
    );
  }
  else {
    $form['actions']['save_continue'] = array(
      '#type' => 'submit',
      '#value' => t('Save and add fields'),
      '#suffix' => l(t('Cancel'), 'admin/structure/commerce-discounts/types'),
      '#submit' => array_merge($submit, array('commerce_discount_discount_type_form_submit')),
      '#weight' => 45,
    );
  }

  $form['#validate'][] = 'commerce_discount_discount_type_form_validate';

  return $form;
}

/**
 * Validation callback for commerce_discount_discount_type_form().
 */
function commerce_discount_discount_type_form_validate($form, &$form_state) {
  $discount_type = $form_state['discount_type'];

  // If saving a new discount type, ensure it has a unique machine name.
  if (empty($discount_type['type'])) {
    if (!commerce_discount_validate_discount_type_unique($form_state['values']['discount_type']['type'])) {
      form_set_error('discount_type][type', t('The machine name specified is already in use.'));
    }
  }
}

/**
 * Form submit handler: save a discount type.
 */
function commerce_discount_discount_type_form_submit($form, &$form_state) {
  $discount_type = $form_state['discount_type'];
  $updated = !empty($discount_type['type']);

  // If a type is set, we should still check to see if a row for the type exists
  // in the database; this is done to accomodate types defined by Features.
  if ($updated) {
    $updated = db_query('SELECT 1 FROM {commerce_discount_type} WHERE type = :type', array(':type' => $discount_type['type']))->fetchField();
  }

  foreach ($form_state['values']['discount_type'] as $key => $value) {
    $discount_type[$key] = $value;
  }

  // Write the discount type to the database.
  $discount_type['is_new'] = !$updated;
  commerce_discount_discount_type_save($discount_type);

  // Redirect based on the button clicked.
  drupal_set_message(t('Discount type saved.'));

  if ($form_state['triggering_element']['#parents'][0] == 'save_continue') {
    $form_state['redirect'] = 'admin/structure/commerce-discounts/types/' . strtr($discount_type['type'], '_', '-') . '/fields';
  }
  else {
    $form_state['redirect'] = 'admin/structure/commerce-discounts/types';
  }
}


<?php

/**
 * @file
 * Provides functionality for inline managing commerce discount offers.
 */

/**
 * IEF add/edit form callback: Returns the offer form to be embedded.
 */
function commerce_discount_inline_offer_form($form, &$form_state) {
  $offer = $form['#entity'];
  // We might have gotten the commerce discount type via ajax, so set it
  // in the commerce discount entity.
  if (!empty($form_state['values']['commerce_discount_fields']['commerce_discount_offer'][LANGUAGE_NONE]['form']['commerce_discount_offer_type'])) {
    $offer->type = $form_state['values']['commerce_discount_fields']['commerce_discount_offer'][LANGUAGE_NONE]['form']['commerce_discount_offer_type'];
  }

  $form['#element_validate'] = array(
    'commerce_discount_inline_offer_form_validate',
  );
  $form['#element_submit'] = array(
    'commerce_discount_inline_offer_form_submit',
  );
  $options = array();
  foreach (commerce_discount_offer_types() as $type => $info) {
    $options[$type] = $info['label'];
  }
  $form['commerce_discount_offer_type'] = array(
    '#title' => t('Choose offer type'),
    '#type' => 'radios',
    '#options' => $options,
    '#required' => TRUE,
    '#default_value' => $offer->type,
    '#ajax' => array(
       'callback' => 'commerce_discount_inline_offer_form_ajax',
       'wrapper' => $form['#ief_wrapper'],
    ),
  );

  field_attach_form('commerce_discount_offer', $offer, $form, $form_state, LANGUAGE_NONE);
  return $form;
}

/**
 * Ajax callback for reloading the inline offer form.
 */
function commerce_discount_inline_offer_form_ajax($form, $form_state) {
  $parents = array_slice($form_state['triggering_element']['#parents'], 0, -2);
  return drupal_array_get_nested_value($form, $parents);
}

/**
 * IEF add/edit form validation callback.
 */
function commerce_discount_inline_offer_form_validate($form, &$form_state) {
  $offer = $form['#entity'];
  field_attach_form_validate('commerce_discount_offer', $offer, $form, $form_state);
}

/**
 * IEF add/edit form submit callback: Modifies the passed-in offer before it
 * is saved.
 */
function commerce_discount_inline_offer_form_submit($form, &$form_state) {
  $offer = $form['#entity'];
  field_attach_submit('commerce_discount_offer', $offer, $form, $form_state);
}

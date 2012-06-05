<?php

/**
 * @file
 * Rules integration for the Commerce Discount module.
 */

/**
 * Implements hook_rules_action_info().
 */
function commerce_discount_rules_action_info() {
  $types = commerce_discount_offer_types();
  $items = array();

  foreach (array('fixed_amount', 'percentage') as $type) {

    $items[$types[$type]['action']] = array(
      'label' => $types[$type]['label'],
      'parameter' => array(
        'entity' => array(
          'label' => t('Entity'),
          'type' => 'entity',
          'wrapped' => TRUE,
        ),
        'commerce_discount' => array(
          'label' => t('Commerce discount'),
          'type' => 'token',
          'options list' => 'commerce_discount_entity_list',
        ),
      ),
      'group' => t('Commerce Discount'),
      'base' => $types[$type]['action'],
    );
  }

  return $items;
}


/**
 * Rules action: Apply fixed amount discount.
 */
function commerce_discount_fixed_amount(EntityDrupalWrapper $wrapper, $discount_name) {
  $discount_wrapper = entity_metadata_wrapper('commerce_discount', $discount_name);
}

/**
 * Rules action: Apply precentage discount.
 */
function commerce_discount_percentage(EntityDrupalWrapper $wrapper, $discount_name) {
  $discount_wrapper = entity_metadata_wrapper('commerce_discount', $discount_name);
}


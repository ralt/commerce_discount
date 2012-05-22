<?php

/**
 * @file
 * Hooks provided by the Commerce Discount module.
 */

/**
 * Defines the types of discounts available for creation on the site.
 *
 * Each defined type is a bundle of the commerce_discount entity type, hence
 * able to have its own fields attached.
 *
 * The discount type array structure includes the following keys:
 * - label: a translatable, human-readable discount type label.
 * - event: the machine name of the rules event used to apply a discount of
 *   the defined type.
 * - entity type: The type of entity to which the discount will be applied.
 *
 * @return
 *   An array of discount type arrays keyed by the machine name of the type.
 */
function hook_commerce_discount_type_info() {
  $types = array();
  $types['order_discount'] = array(
    'label' => t('Order Discount'),
    'event' => 'commerce_discount_calculate_order_discount',
    'entity type' => 'commerce_order',
  );
  $types['product_discount'] = array(
    'label' => t('Product Discount'),
    'event' => 'commerce_product_calculate_sell_price',
    'entity type' => 'commerce_product',
  );

  return $types;
}

/**
 * Defines the types of discount offers available for creation on the site.
 *
 * Each defined type is a bundle of the commerce_discount_offer entity type,
 * hence able to have its own fields attached.
 *
 * The discount offer type array structure includes the following keys:
 * - label: a translatable, human-readable discount offer type label.
 * - callback: the function callback used to apply a discount offer of the
 *   defined type to an entity.
 *
 * @return
 *   An array of discount offer type arrays keyed by the machine name of the
 *   type.
 */
function hook_commerce_discount_offer_type_info() {
  $types = array();
  $types['fixed_amount'] = array(
    'label' => t('$ off'),
    'callback' => 'commerce_discount_fixed_amount_offer_apply',
  );
  $types['percentage'] = array(
    'label' => t('% off'),
    'callback' => 'commerce_discount_percentage_offer_apply',
  );

  return $types;
}

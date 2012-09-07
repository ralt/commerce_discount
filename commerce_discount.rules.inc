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
 * Implements hook_rules_event_info().
 */
function commerce_discount_rules_event_info() {
  $items = array(
    'commerce_discount_order' => array(
      'label' => t('Apply a discount to a given order'),
      'group' => t('Commerce - discount'),
      'variables' => entity_rules_events_variables('commerce_order', t('Order', array(), array('context' => 'a drupal commerce order'))),
      'access callback' => 'commerce_order_rules_access',
    ),
  );

  return $items;
}


/**
 * Rules action: Apply fixed amount discount.
 */
function commerce_discount_fixed_amount(EntityDrupalWrapper $wrapper, $discount_name) {
  $discount_wrapper = entity_metadata_wrapper('commerce_discount', $discount_name);
  switch ($discount_wrapper->getBundle()) {
    case 'order_discount':
      // Set reference to the discount.
      // TODO: It doesn't work with the wrapper.
      $order = $wrapper->value();
      $delta = $wrapper->commerce_discounts->count();
      $order->commerce_discounts[LANGUAGE_NONE][$delta]['target_id'] = $discount_wrapper->discount_id->value();

      if (!$line_item = commerce_discount_get_fixed_order_discount_line_item($order, $discount_wrapper->value())) {
        return;
      }

      commerce_line_item_save($line_item);
      $wrapper->commerce_line_items[] = $line_item;

      // Updating the commerce order calculate price; For some reason, without
      // the following call to commerce_order_calculate_total() the discount
      // components aren't re-added after they get deleted in
      // commerce_discount_commerce_order_presave().
      commerce_order_calculate_total($order);
      break;

    case 'product_discount':
      // Check whether this discount was already added as a price component.
      $components = $wrapper->commerce_unit_price->data->value();
      foreach ($components as $component) {
        if (!empty($component['price']['data']['discount_name']) && $component['price']['data']['discount_name'] == $discount_wrapper->getIdentifier()) {
          return;
        }
      }

      $unit_price = commerce_price_wrapper_value($wrapper, 'commerce_unit_price', TRUE);
      $current_amount = $unit_price['amount'];
      $discount_price = $discount_wrapper->commerce_discount_offer->commerce_fixed_amount->value();
      $discount_amount = $discount_price['amount'];

      // Calculate the updated amount and create a price array representing the
      // difference between it and the current amount.
      $updated_amount = commerce_round(COMMERCE_ROUND_HALF_UP, $current_amount - $discount_amount);

      $difference = array(
        'amount' => $updated_amount - $current_amount,
        'currency_code' => $unit_price['currency_code'],
        'data' => array('discount_name' => $discount_wrapper->getIdentifier()),
      );

      // Set the amount of the unit price and add the difference as a component.
      $wrapper->commerce_unit_price->amount = $updated_amount;

      $wrapper->commerce_unit_price->data = commerce_price_component_add(
        $wrapper->commerce_unit_price->value(),
        'discount',
        $difference,
        TRUE
      );
      break;
  }
}

/**
 * Rules action: Apply percentage discount.
 */
function commerce_discount_percentage(EntityDrupalWrapper $wrapper, $discount_name) {
  $discount_wrapper = entity_metadata_wrapper('commerce_discount', $discount_name);
  switch ($discount_wrapper->getBundle()) {
    case 'order_discount':
      if (!$wrapper->commerce_line_items->value()) {
        return;
      }

      // Set reference to the discount.
      // TODO: It doesn't work with the wrapper.
      $order = $wrapper->value();
      $delta = $wrapper->commerce_discounts->count();
      $order->commerce_discounts[LANGUAGE_NONE][$delta]['target_id'] = $discount_wrapper->discount_id->value();

      if (!$line_item = commerce_discount_get_percentage_order_discount_line_item($order, $discount_wrapper->value())) {
        return;
      }

      commerce_line_item_save($line_item);
      $wrapper->commerce_line_items[] = $line_item;

      // Updating the commerce order calculate price; For some reason, without
      // the following call to commerce_order_calculate_total() the discount
      // components aren't re-added after they get deleted in
      // commerce_discount_commerce_order_presave().
      commerce_order_calculate_total($order);
      break;

    case 'product_discount':
      commerce_discount_apply_percentage_discount($wrapper, $discount_wrapper);
      break;
  }
}

/**
 * Apply percentage discount to a line item.
 *
 * @param $line_item_wrapper
 *   Wrapped commerce_line_item to apply the discount to.
 * @param $discount_wrapper
 *   Wrapped commerce_discount to apply to the line item.
 */
function commerce_discount_apply_percentage_discount($line_item_wrapper, $discount_wrapper) {
  $unit_price = $line_item_wrapper->commerce_unit_price->value();

  // Make sure the same discount isn't added twice.
  foreach ($unit_price['data']['components'] as $component) {
    if (!empty($component['price']['data']['discount_name']) && $component['price']['data']['discount_name'] == $discount_wrapper->getIdentifier()) {
      return;
    }
  }

  $rate = $discount_wrapper->commerce_discount_offer->commerce_percentage->value();
  if ($rate > 1) {
    $rate = $rate / 100;
  }
  else {
    return;
  }

  $unit_price = commerce_price_wrapper_value($line_item_wrapper, 'commerce_unit_price', TRUE);
  $current_amount = $unit_price['amount'];
  $discounted_amount = $current_amount - ($current_amount * $rate);

  // Calculate the updated amount and create a price array representing the
  // difference between it and the current amount.
  $updated_amount = commerce_round(COMMERCE_ROUND_HALF_UP, $discounted_amount);

  $difference = array(
    'amount' => $updated_amount - $current_amount,
    'currency_code' => $unit_price['currency_code'],
    'data' => array('discount_name' => $discount_wrapper->getIdentifier()),
  );

  // Set the amount of the unit price and add the difference as a component.
  $line_item_wrapper->commerce_unit_price->amount = $updated_amount;

  $line_item_wrapper->commerce_unit_price->data = commerce_price_component_add(
    $line_item_wrapper->commerce_unit_price->value(),
    'discount',
    $difference,
    TRUE
  );
}

/**
 * Creates a new discount line item populated with the proper discount values.
 *
 * @param $order
 *   The commerce order entity.
 * @param $discount
 *   The commerce discount entity.
 *
 * @return
 *   An unsaved line item entity.
 */
function commerce_discount_get_percentage_order_discount_line_item($order, $discount) {
  // Create a new line item.
  $discount_line_item = entity_create('commerce_line_item', array(
    'type' => 'commerce_discount',
    'order_id' => $order->order_id,
    'quantity' => 1,
  ));
  $discount_line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $discount_line_item);
  $discount_wrapper = entity_metadata_wrapper('commerce_discount', $discount);
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

  $rate = $discount_wrapper->commerce_discount_offer->commerce_percentage->value();
  if ($rate > 1) {
    $rate = $rate / 100;
  }
  else {
    return;
  }
  // Initialize a line item unit price to amount 0 with the currency from the
  // order.
  $discount_line_item_wrapper->commerce_unit_price->amount = 0;
  $discount_line_item_wrapper->commerce_unit_price->currency_code = $order_wrapper->commerce_order_total->currency_code->value();

  // Reset the data array of the line item total field to only include a
  // base price component, set the currency code from the order.
  $base_price = array(
    'amount' => 0,
    'currency_code' => $order_wrapper->commerce_order_total->currency_code->value(),
    'data' => array(),
  );
  $discount_line_item_wrapper->commerce_unit_price->data = commerce_price_component_add($base_price, 'base_price', $base_price, TRUE);

  // Loop the line items of the order and calculate the total discount.
  foreach ($order_wrapper->commerce_line_items->value() as $line_item) {
    $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
    $line_item_unit_price = commerce_price_wrapper_value($line_item_wrapper, 'commerce_unit_price', TRUE);
    $line_item_current_amount = $line_item_unit_price['amount'];

    $discount_line_item_price = commerce_price_wrapper_value($discount_line_item_wrapper, 'commerce_unit_price', TRUE);
    $discount_line_item_current_amount = $discount_line_item_price['amount'];

    $discounted_amount = $discount_line_item_current_amount - ($line_item_current_amount * $rate * $line_item->quantity);

    // Calculate the updated amount and create a price array representing the
    // difference between it and the current amount.
    $updated_amount = commerce_round(COMMERCE_ROUND_HALF_UP, $discounted_amount);

    $difference = array(
      'amount' => $updated_amount - $discount_line_item_current_amount,
      'currency_code' => $line_item_unit_price['currency_code'],
      'data' => array('discount_name' => $discount_wrapper->getIdentifier()),
    );

    // Set the amount of the unit price and add the difference as a component.
    $discount_line_item_wrapper->commerce_unit_price->amount = $updated_amount;

    $discount_line_item_wrapper->commerce_unit_price->data = commerce_price_component_add(
      $discount_line_item_wrapper->commerce_unit_price->value(),
      'discount',
      $difference,
      TRUE
    );
  }

  return $discount_line_item_wrapper->value();
}

/**
 * Creates a new discount line item populated with the proper discount values.
 *
 * @param $order
 *   The commerce order entity.
 * @param $discount
 *   The commerce discount entity.
 *
 * @return
 *   An unsaved line item entity.
 */
function commerce_discount_get_fixed_order_discount_line_item($order, $discount) {
  // Create a new line item.
  $line_item = entity_create('commerce_line_item', array(
    'type' => 'commerce_discount',
    'order_id' => $order->order_id,
    'quantity' => 1,
  ));

  $wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
  $discount_wrapper = entity_metadata_wrapper('commerce_discount', $discount);

  $price_component = $discount_wrapper->commerce_discount_offer->commerce_fixed_amount->value();
  // Make sure the amount is negative.
  $price_component['amount'] = $price_component['amount'] * -1;
  $data = commerce_price_component_add($price_component, 'discount', $price_component, TRUE, FALSE);
  $data['components'][0]['price']['data']['discount_name'] = $discount->name;
  $wrapper->commerce_unit_price->amount = $price_component['amount'];
  $wrapper->commerce_unit_price->data = $data;

  return $wrapper->value();
}
